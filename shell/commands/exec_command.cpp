#include "exec_command.hpp" // Corrected include to be local
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h> // For fork, execvp, pipe, dup2, close
#include <sys/wait.h> // For waitpid
#include <cstdio> // For popen, pclose, fgets if we were to use popen (alternative)
                  // For this implementation, using pipe/fork/exec, it's not strictly needed
                  // but often included with system calls.
#include <cstring> // For strerror
#include <poll.h>
#include <cerrno>
#include <stdexcept> // For std::runtime_error

// Define BUFFER_SIZE for reading from pipes
#define BUFFER_SIZE 1024

namespace {
// Own descriptors only while launch is being prepared. Successful fork hands
// them to the existing parent/child paths; failures and unwinding close them.
class LaunchPipeGuard {
public:
    LaunchPipeGuard(int (&out)[2], int (&err)[2]) noexcept : out_(out), err_(err) {}
    LaunchPipeGuard(const LaunchPipeGuard&) = delete;
    LaunchPipeGuard& operator=(const LaunchPipeGuard&) = delete;
    LaunchPipeGuard(LaunchPipeGuard&&) = delete;
    LaunchPipeGuard& operator=(LaunchPipeGuard&&) = delete;
    ~LaunchPipeGuard() noexcept {
        if (!owns_) return;
        for (int fd : out_) if (fd >= 0) close(fd);
        for (int fd : err_) if (fd >= 0) close(fd);
    }
    void release() noexcept { owns_ = false; }
private:
    int (&out_)[2];
    int (&err_)[2];
    bool owns_ = true;
};
} // namespace

namespace Neurodeck {

ExecCommand::ExecCommand() {}

std::string ExecCommand::name() const {
    return "exec";
}

std::string ExecCommand::description() const {
    return "Executes a system command and captures its output. Usage: exec <command> [args...]";
}

void ExecCommand::run(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Usage: exec <command> [args...]" << std::endl;
        return;
    }
    if (args.size() < 2) {
        std::cerr << "Usage: " << args[0] << " <command> [args...]" << std::endl;
        return;
    }

    // Prepare arguments for execvp
    // The first argument to execvp is the command name.
    // The second argument is a null-terminated array of C-style strings (char*).
    // The first element of this array is also the command name.
    std::vector<char*> c_args;
    for (size_t i = 1; i < args.size(); ++i) {
        c_args.push_back(const_cast<char*>(args[i].c_str()));
    }
    c_args.push_back(nullptr); // Null-terminate the array

    // Pipes for capturing stdout and stderr
    int stdout_pipe[2] = {-1, -1};
    int stderr_pipe[2] = {-1, -1};
    LaunchPipeGuard launch_pipes(stdout_pipe, stderr_pipe);

    if (pipe(stdout_pipe) == -1) {
        std::cerr << "Failed to create stdout pipe: " << strerror(errno) << std::endl;
        return;
    }
    if (pipe(stderr_pipe) == -1) {
        std::cerr << "Failed to create stderr pipe: " << strerror(errno) << std::endl;
        return;
    }

    pid_t pid = fork();
    if (pid >= 0) launch_pipes.release();

    if (pid == -1) {
        // Fork failed
        std::cerr << "Failed to fork process: " << strerror(errno) << std::endl;
        return;
    } else if (pid == 0) {
        // Child process

        // Redirect stdout to the write end of stdout_pipe
        dup2(stdout_pipe[1], STDOUT_FILENO);
        // Redirect stderr to the write end of stderr_pipe
        dup2(stderr_pipe[1], STDERR_FILENO);

        // Close unused pipe ends in child
        close(stdout_pipe[0]); // Read end of stdout_pipe
        close(stdout_pipe[1]); // Write end of stdout_pipe (already duplicated)
        close(stderr_pipe[0]); // Read end of stderr_pipe
        close(stderr_pipe[1]); // Write end of stderr_pipe (already duplicated)
        
        // Execute the command
        execvp(c_args[0], c_args.data());
        
        // If execvp returns, it means an error occurred
        std::cerr << "Failed to execute command '" << c_args[0] << "': " << strerror(errno) << std::endl;
        // Ensure child process exits if execvp fails to prevent it from continuing as the parent's code.
        // Write error to the original stderr (or our stderr_pipe if dup2 was successful)
        // then exit.
        _exit(EXIT_FAILURE); 


    } else {
        // Parent process

        // Close unused write ends of pipes in parent
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        std::string captured_stdout;
        std::string captured_stderr;
        char buffer[BUFFER_SIZE];
        // A sole reader services either pipe as it becomes ready. POLLHUP still
        // requires draining buffered bytes before read() returns EOF.
        pollfd streams[2] = {{stdout_pipe[0], POLLIN, 0},
                             {stderr_pipe[0], POLLIN, 0}};
        std::string* captures[2] = {&captured_stdout, &captured_stderr};
        const char* labels[2] = {"stdout", "stderr"};
        int open_streams = 2;
        while (open_streams > 0) {
            int ready = poll(streams, 2, -1);
            if (ready < 0) {
                if (errno == EINTR) continue;
                std::cerr << "Error polling child output: " << strerror(errno) << std::endl;
                break;
            }
            for (int i = 0; i < 2; ++i) {
                if (streams[i].fd < 0 || streams[i].revents == 0) continue;
                const ssize_t bytes_read = read(streams[i].fd, buffer, BUFFER_SIZE - 1);
                if (bytes_read > 0) {
                    captures[i]->append(buffer, static_cast<size_t>(bytes_read));
                    continue;
                }
                if (bytes_read < 0 && errno == EINTR) continue;
                if (bytes_read < 0) {
                    std::cerr << "Error reading from child " << labels[i] << ": "
                              << strerror(errno) << std::endl;
                }
                close(streams[i].fd);
                streams[i].fd = -1;
                --open_streams;
            }
        }
        for (const auto& stream : streams) {
            if (stream.fd >= 0) close(stream.fd);
        }

        // Wait for child process to complete
        int status = 0;
        pid_t waited;
        do {
            waited = waitpid(pid, &status, 0);
        } while (waited == -1 && errno == EINTR);
        const int wait_error = waited == -1 ? errno : 0;

        if (!captured_stdout.empty()) {
            std::cout << "Stdout:\n" << captured_stdout << std::endl;
        }
        if (!captured_stderr.empty()) {
            // If execvp failed in child, that error message might be here.
            // Or any other stderr output from the command.
            std::cerr << "Stderr:\n" << captured_stderr << std::endl;
        }

        if (waited != pid) {
            std::cerr << "Failed to wait for child process: "
                      << strerror(wait_error) << std::endl;
            return;
        }
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                std::cerr << "Command exited with status " << exit_code << std::endl;
            }
        } else if (WIFSIGNALED(status)) {
            std::cerr << "Command killed by signal " << WTERMSIG(status) << std::endl;
        }
    }
}

} // namespace Neurodeck
