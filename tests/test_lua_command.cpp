#include "gtest/gtest.h"
#include "command_registry.hpp" // Corrected include path
#include "commands/lua_command.hpp" // Corrected include path
#include "lua_manager.hpp" // Corrected include path
#include <iostream> // For std::cout, std::cerr (used by tests, not interceptor directly)
#include <sstream>  // For std::ostringstream (used by tests, not interceptor directly)
#include <string>
#include <vector>
#include <unistd.h> // For pipe, dup, dup2, close, read, STDOUT_FILENO, STDERR_FILENO
#include <fcntl.h>  // For fcntl, F_GETFL, F_SETFL, O_NONBLOCK
#include <stdexcept> // For std::runtime_error
#include <cstring> // For strerror
#include <cerrno>  // For errno

// Helper class to capture stdout/stderr using file descriptors
class OutputInterceptor {
public:
    OutputInterceptor() {
        // Save original stdout and stderr file descriptors
        originalStdoutFd = dup(STDOUT_FILENO);
        originalStderrFd = dup(STDERR_FILENO);

        if (originalStdoutFd == -1 || originalStderrFd == -1) {
            // Consider logging errno here
            throw std::runtime_error(std::string("Failed to dup original stdout/stderr: ") + strerror(errno));
        }

        // Create pipes for stdout and stderr
        if (pipe(stdoutPipe) == -1 || pipe(stderrPipe) == -1) {
            // Consider logging errno here
            // Clean up any fds/pipes already created if possible
            if(stdoutPipe[0] != -1) close(stdoutPipe[0]);
            if(stdoutPipe[1] != -1) close(stdoutPipe[1]);
            if(originalStdoutFd != -1) close(originalStdoutFd);
            if(originalStderrFd != -1) close(originalStderrFd);
            throw std::runtime_error(std::string("Failed to create pipes: ") + strerror(errno));
        }

        // Redirect stdout and stderr to the write ends of the pipes
        if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1 || dup2(stderrPipe[1], STDERR_FILENO) == -1) {
            // Consider logging errno here
            // Attempt to restore original FDs and clean up before throwing
            dup2(originalStdoutFd, STDOUT_FILENO); // Best effort
            dup2(originalStderrFd, STDERR_FILENO); // Best effort
            close(stdoutPipe[0]); close(stdoutPipe[1]);
            close(stderrPipe[0]); close(stderrPipe[1]);
            close(originalStdoutFd); close(originalStderrFd);
            throw std::runtime_error(std::string("Failed to dup2 pipes to stdout/stderr: ") + strerror(errno));
        }

        // Close the write ends in the parent, they are now managed by stdout/stderr
        // and duplicated in STDOUT_FILENO, STDERR_FILENO
        close(stdoutPipe[1]); stdoutPipe[1] = -1; // Mark as closed
        close(stderrPipe[1]); stderrPipe[1] = -1; // Mark as closed


        // Make read ends non-blocking
        setNonBlocking(stdoutPipe[0]);
        setNonBlocking(stderrPipe[0]);
    }

    ~OutputInterceptor() {
        // Restore original stdout and stderr
        if (originalStdoutFd != -1) {
            dup2(originalStdoutFd, STDOUT_FILENO);
            close(originalStdoutFd);
        }
        if (originalStderrFd != -1) {
            dup2(originalStderrFd, STDERR_FILENO);
            close(originalStderrFd);
        }

        // Close read ends of the pipes
        if (stdoutPipe[0] != -1) close(stdoutPipe[0]);
        if (stderrPipe[0] != -1) close(stderrPipe[0]);
        // Write ends should have been closed in constructor or by the system if redirection failed
        if (stdoutPipe[1] != -1) close(stdoutPipe[1]); 
        if (stderrPipe[1] != -1) close(stderrPipe[1]);
    }

    std::string getStdout() {
        return readFromPipe(stdoutPipe[0]);
    }

    std::string getStderr() {
        return readFromPipe(stderrPipe[0]);
    }

    void clear() {
        readFromPipe(stdoutPipe[0]); // Drain stdout
        readFromPipe(stderrPipe[0]); // Drain stderr
    }

private:
    int stdoutPipe[2] = {-1, -1}; // stdoutPipe[0] is read end, stdoutPipe[1] is write end
    int stderrPipe[2] = {-1, -1}; // stderrPipe[0] is read end, stderrPipe[1] is write end
    int originalStdoutFd = -1;
    int originalStderrFd = -1;

    void setNonBlocking(int fd) {
        if (fd == -1) return;
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) {
            throw std::runtime_error(std::string("fcntl F_GETFL failed: ") + strerror(errno));
        }
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            throw std::runtime_error(std::string("fcntl F_SETFL O_NONBLOCK failed: ") + strerror(errno));
        }
    }

    std::string readFromPipe(int fd) {
        if (fd == -1) return "";
        std::string output;
        char buffer[256]; 
        ssize_t bytes_read;

        while (true) {
            bytes_read = read(fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                output += buffer;
            } else if (bytes_read == 0) { 
                break;
            } else { 
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                } else {
                    // This could be an actual error, or just that the write end was closed.
                    // For test robustness, we'll just stop reading.
                    // std::cerr << "Read error on pipe: " << strerror(errno) << std::endl;
                    break; 
                }
            }
        }
        return output;
    }
};

// Test fixture for LuaCommand tests
class LuaCommandTest : public ::testing::Test {
protected:
    Neurodeck::LuaCommand luaCommand;
    OutputInterceptor interceptor;

    void runCommand(const std::vector<std::string>& args) {
        luaCommand.run(args);
    }
};

TEST_F(LuaCommandTest, NameAndDescription) {
    EXPECT_EQ(luaCommand.name(), "lua");
    EXPECT_FALSE(luaCommand.description().empty());
}

TEST_F(LuaCommandTest, SimplePrint) {
    runCommand({"lua", "print('Hello from Lua')"});
    std::string captured_stdout = interceptor.getStdout();
    std::string captured_stderr = interceptor.getStderr();
    if (!captured_stderr.empty()) {
        // Print to std::cout which gets captured by CTest / test runner log
        std::cout << "[DEBUG_STDERR_SimplePrint] " << captured_stderr << std::endl;
    }
    EXPECT_EQ(captured_stdout, "Hello from Lua\n");
    EXPECT_TRUE(captured_stderr.empty());
}

TEST_F(LuaCommandTest, MathLib) {
    runCommand({"lua", "print(math.sqrt(16))"});
    // Lua's print might format whole numbers as "4.0" or "4". 
    // Let's check for either, then ensure it ends with a newline.
    std::string output = interceptor.getStdout();
    bool correct_output = (output == "4\n" || output == "4.0\n");
    EXPECT_TRUE(correct_output) << "Expected '4\\n' or '4.0\\n', got '" << output << "'";
    EXPECT_TRUE(interceptor.getStderr().empty());
}

TEST_F(LuaCommandTest, StringLib) {
    runCommand({"lua", "print(string.upper('test'))"});
    EXPECT_EQ(interceptor.getStdout(), "TEST\n");
    EXPECT_TRUE(interceptor.getStderr().empty());
}

TEST_F(LuaCommandTest, TableLib) {
    // Simple table usage, e.g., creating and accessing a table
    runCommand({"lua", "local t = {a=1, b=2}; print(t.a + t.b)"});
    EXPECT_EQ(interceptor.getStdout(), "3\n"); // Lua prints numbers as numbers, not integers necessarily
    EXPECT_TRUE(interceptor.getStderr().empty());
}

TEST_F(LuaCommandTest, ShellRunCommand) {
    // This test is platform-dependent and relies on 'echo' command being available.
    // It also assumes shell.run prints output to stdout, which it might not directly.
    // LuaManager's l_shell_run returns status, doesn't print output itself.
    // Let's test a command that should succeed.
    // The current l_shell_run in LuaManager only returns the exit code.
    // To test output, Lua script itself must print the result of shell.run or its output.
    // For now, let's test if it can execute and if the return value is what we expect (0 for success).
    runCommand({"lua", "local ret = shell.run('echo test_shell_run'); print(ret)"});
    // The output 'test_shell_run' from echo will go to the main program's stdout,
    // not necessarily captured by Lua's print.
    // The `print(ret)` will print the return code of `system("echo test_shell_run")`.
    // We expect this to be 0.
    // The actual output of "echo test_shell_run" will also appear on stdout.
    // So, the stdout will be "test_shell_run\n0\n" (approximately)
    std::string full_stdout = interceptor.getStdout();
    // Depending on system, `echo` might actually be an internal shell command.
    // `system("echo ...")` might behave slightly differently or involve an actual process.
    // We'll check if the output *contains* the expected parts.
    EXPECT_TRUE(full_stdout.find("test_shell_run") != std::string::npos);
    EXPECT_TRUE(full_stdout.find("0") != std::string::npos); // Return code 0
    EXPECT_TRUE(interceptor.getStderr().empty());
}


TEST_F(LuaCommandTest, ShellRunCommandFailure) {
    // Test a command that should fail and return a non-zero status.
    // "this_command_should_not_exist_anywhere" is unlikely to be a real command.
    runCommand({"lua", "local ret = shell.run('this_command_should_not_exist_anywhere'); print(ret)"});
    // The return code will be non-zero. The exact value can vary.
    // We check that the output is not "0\n".
    // stderr from the failing command might appear on the main stderr, not Lua's.
    // LuaManager's l_shell_run doesn't capture command's stderr to Lua.
    std::string stdout_content = interceptor.getStdout();
    EXPECT_NE(stdout_content, "0\n"); 
    EXPECT_NE(stdout_content, ""); // Should print something (the non-zero return code)
    // We can't easily check stderr here because system() behavior w.r.t stderr is complex.
    // The LuaManager itself doesn't report an error for a non-zero return from shell.run.
}


TEST_F(LuaCommandTest, InvalidLuaSyntax) {
    runCommand({"lua", "print('hello'"}); // Missing closing parenthesis
    EXPECT_TRUE(interceptor.getStdout().empty());
    // LuaManager prints errors to std::cerr
    std::string stderr_output = interceptor.getStderr();
    EXPECT_FALSE(stderr_output.empty());
    // Example error: "[string "print('hello'"]:1: unexpected symbol near '<eof>'"
    EXPECT_TRUE(stderr_output.find("unexpected symbol") != std::string::npos || 
                stderr_output.find("syntax error") != std::string::npos ||
                stderr_output.find("<eof>") != std::string::npos);
}

TEST_F(LuaCommandTest, LuaRuntimeError) {
    runCommand({"lua", "error('test error')"});
    EXPECT_TRUE(interceptor.getStdout().empty());
    std::string stderr_output = interceptor.getStderr();
    EXPECT_FALSE(stderr_output.empty());
    // Example error: "[string "error('test error')"]:1: test error"
    EXPECT_TRUE(stderr_output.find("test error") != std::string::npos);
}

TEST_F(LuaCommandTest, NoArguments) {
    runCommand({"lua"});
    std::string captured_stdout = interceptor.getStdout();
    std::string captured_stderr = interceptor.getStderr();
    if (!captured_stderr.empty()) {
        std::cout << "[DEBUG_STDERR_NoArguments] " << captured_stderr << std::endl;
    }
    EXPECT_TRUE(captured_stdout.empty());
    EXPECT_FALSE(captured_stderr.empty());
    EXPECT_NE(captured_stderr.find("Usage: lua"), std::string::npos);
}

TEST_F(LuaCommandTest, EmptyCodeString) {
    // This test case depends on how the shell passes an empty quoted string.
    // If the shell passes "" as an argument:
    runCommand({"lua", ""}); // Explicitly pass an empty string argument
    std::string captured_stdout = interceptor.getStdout();
    std::string captured_stderr = interceptor.getStderr();
    if (!captured_stderr.empty()) {
        std::cout << "[DEBUG_STDERR_EmptyCodeString] " << captured_stderr << std::endl;
    }
    EXPECT_TRUE(captured_stdout.empty());
    EXPECT_FALSE(captured_stderr.empty());
    EXPECT_NE(captured_stderr.find("No Lua code provided."), std::string::npos);
}

TEST_F(LuaCommandTest, MultipleStringArgumentsConcatenated) {
    runCommand({"lua", "print('Hello,", "World!')"}); // "lua" "print('Hello," "World!')"
    EXPECT_EQ(interceptor.getStdout(), "Hello, World!\n");
    EXPECT_TRUE(interceptor.getStderr().empty());
}

// This test assumes LUA_FOUND_IN_CMAKE is defined. 
// If not, LuaManager is a stub and tests will behave differently.
// The LuaManager stub prints to cerr when executeString is called.
#ifndef LUA_FOUND_IN_CMAKE
TEST_F(LuaCommandTest, LuaNotAvailableStubBehavior) {
    // This test is specific to the stub implementation of LuaManager
    interceptor.clear(); // Clear any previous output
    runCommand({"lua", "print('This will not run if Lua is a stub')"});
    EXPECT_TRUE(interceptor.getStdout().empty()); // Lua's print won't run
    // The stub LuaManager::executeString prints a message to std::cerr
    std::string stderr_output = interceptor.getStderr();
    EXPECT_FALSE(stderr_output.empty());
    EXPECT_NE(stderr_output.find("Lua is not available"), std::string::npos);
}
#endif
