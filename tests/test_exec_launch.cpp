#include "commands/exec_command.hpp"
#include <gtest/gtest.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

namespace {

struct LaunchFailure {
    int failing_pipe_call;
    bool fail_fork;
    int pipe_calls = 0;
    int fork_calls = 0;
    std::array<int, 4> opened_fds{};
    std::size_t opened_count = 0;
};

LaunchFailure* active_failure = nullptr;

class InjectLaunchFailure {
public:
    explicit InjectLaunchFailure(LaunchFailure& failure) { active_failure = &failure; }
    InjectLaunchFailure(const InjectLaunchFailure&) = delete;
    InjectLaunchFailure& operator=(const InjectLaunchFailure&) = delete;
    ~InjectLaunchFailure() { active_failure = nullptr; }
};

class CaptureStream {
public:
    explicit CaptureStream(std::ostream& stream)
        : stream_(stream), original_(stream_.rdbuf(output_.rdbuf())) {}
    CaptureStream(const CaptureStream&) = delete;
    CaptureStream& operator=(const CaptureStream&) = delete;
    ~CaptureStream() { stream_.rdbuf(original_); }

    std::string str() const { return output_.str(); }

private:
    std::ostream& stream_;
    std::ostringstream output_;
    std::streambuf* original_;
};

void expectLaunchFailure(LaunchFailure failure, int pipe_calls, int fork_calls,
                         std::size_t opened_count, const char* diagnostic) {
    Neurodeck::ExecCommand command;
    {
        CaptureStream captured(std::cerr);
        {
            InjectLaunchFailure inject(failure);
            command.run({"exec", "/bin/true"});
        }
        const int expected_error = failure.fail_fork ? EAGAIN : EMFILE;
        EXPECT_EQ(captured.str(), std::string(diagnostic) + std::strerror(expected_error) + "\n");
    }
    EXPECT_EQ(failure.pipe_calls, pipe_calls);
    EXPECT_EQ(failure.fork_calls, fork_calls);
    ASSERT_EQ(failure.opened_count, opened_count);
    for (std::size_t i = 0; i < failure.opened_count; ++i) {
        const int fd = failure.opened_fds[i];
        errno = 0;
        const int result = fcntl(fd, F_GETFD);
        const int error = errno;
        EXPECT_EQ(result, -1) << "pipe fd " << fd;
        EXPECT_EQ(error, EBADF) << "pipe fd " << fd;
    }

    CaptureStream captured(std::cerr);
    command.run({"exec", "/bin/sh", "-c", "printf recovered >&2"});
    EXPECT_EQ(captured.str(), "Stderr:\nrecovered\n");
}

struct WaitFailure {
    int error;
    int calls = 0;
    pid_t child = -1;
    pid_t reaped = -1;
};

WaitFailure* active_wait_failure = nullptr;

class InjectWaitFailure {
public:
    explicit InjectWaitFailure(WaitFailure& failure) { active_wait_failure = &failure; }
    InjectWaitFailure(const InjectWaitFailure&) = delete;
    InjectWaitFailure& operator=(const InjectWaitFailure&) = delete;
    ~InjectWaitFailure() { active_wait_failure = nullptr; }
};

} // namespace

extern "C" int __real_pipe(int* fds);
extern "C" pid_t __real_fork();
extern "C" pid_t __real_waitpid(pid_t child, int* status, int options);

extern "C" int __wrap_pipe(int* fds) {
    auto* failure = active_failure;
    if (failure && ++failure->pipe_calls == failure->failing_pipe_call) {
        errno = EMFILE;
        return -1;
    }
    const int result = __real_pipe(fds);
    if (failure && result == 0 && failure->opened_count + 2 <= failure->opened_fds.size()) {
        failure->opened_fds[failure->opened_count++] = fds[0];
        failure->opened_fds[failure->opened_count++] = fds[1];
    }
    return result;
}

extern "C" pid_t __wrap_fork() {
    auto* failure = active_failure;
    if (failure) {
        ++failure->fork_calls;
        if (failure->fail_fork) {
            errno = EAGAIN;
            return -1;
        }
    }
    return __real_fork();
}

extern "C" pid_t __wrap_waitpid(pid_t child, int* status, int options) {
    auto* failure = active_wait_failure;
    if (!failure) {
        return __real_waitpid(child, status, options);
    }

    failure->child = child;
    ++failure->calls;
    if (failure->error == EINTR && failure->calls == 1) {
        errno = EINTR;
        return -1;
    }

    const pid_t result = __real_waitpid(child, status, options);
    if (result == child && child > 0) {
        failure->reaped = result;
        if (failure->error == ECHILD) {
            // Leave the real exit status in place so decoding it on error is observable.
            errno = ECHILD;
            return -1;
        }
    }
    return result;
}

TEST(ExecLaunchTest, ReportsStdoutPipeFailureBeforeFork) {
    expectLaunchFailure({1, false}, 1, 0, 0, "Failed to create stdout pipe: ");
}

TEST(ExecLaunchTest, ClosesStdoutPipeWhenStderrPipeFails) {
    expectLaunchFailure({2, false}, 2, 0, 2, "Failed to create stderr pipe: ");
}

TEST(ExecLaunchTest, ClosesBothPipesWhenForkFails) {
    expectLaunchFailure({0, true}, 2, 1, 4, "Failed to fork process: ");
}

namespace {

void expectWaitFailure(int error, int expected_calls, const std::string& diagnostic) {
    WaitFailure failure{error};
    Neurodeck::ExecCommand command;
    {
        CaptureStream output(std::cout);
        CaptureStream errors(std::cerr);
        {
            InjectWaitFailure inject(failure);
            command.run({"exec", "/bin/sh", "-c", "printf out; printf err >&2; exit 7"});
        }
        EXPECT_EQ(output.str(), "Stdout:\nout\n");
        EXPECT_EQ(errors.str(), "Stderr:\nerr\n" + diagnostic + "\n");
    }

    EXPECT_EQ(failure.calls, expected_calls);
    ASSERT_GT(failure.child, 0);
    EXPECT_EQ(failure.reaped, failure.child);

    // If the retry regresses, reap the finite fixture here and fail the test.
    pid_t remaining;
    do {
        remaining = __real_waitpid(failure.child, nullptr, 0);
    } while (remaining == -1 && errno == EINTR);
    const int wait_error = errno;
    EXPECT_EQ(remaining, -1);
    EXPECT_EQ(wait_error, ECHILD);
}

} // namespace

TEST(ExecWaitTest, RetriesInterruptedWaitAndReportsChildExit) {
    expectWaitFailure(EINTR, 2, "Command exited with status 7");
}

TEST(ExecWaitTest, ReportsTerminalWaitFailureAndPreservesOutput) {
    expectWaitFailure(ECHILD, 1, std::string("Failed to wait for child process: ") +
                     std::strerror(ECHILD));
}
