#include "commands/exec_command.hpp"
#include <gtest/gtest.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>
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

class CaptureError {
public:
    CaptureError() : original_(std::cerr.rdbuf(output_.rdbuf())) {}
    ~CaptureError() { std::cerr.rdbuf(original_); }

    std::string str() const { return output_.str(); }

private:
    std::ostringstream output_;
    std::streambuf* original_;
};

void expectLaunchFailure(LaunchFailure failure, int pipe_calls, int fork_calls,
                         std::size_t opened_count, const char* diagnostic) {
    Neurodeck::ExecCommand command;
    {
        CaptureError captured;
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

    CaptureError captured;
    command.run({"exec", "/bin/sh", "-c", "printf recovered >&2"});
    EXPECT_EQ(captured.str(), "Stderr:\nrecovered\n");
}

} // namespace

extern "C" int __real_pipe(int* fds);
extern "C" pid_t __real_fork();

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

TEST(ExecLaunchTest, ReportsStdoutPipeFailureBeforeFork) {
    expectLaunchFailure({1, false}, 1, 0, 0, "Failed to create stdout pipe: ");
}

TEST(ExecLaunchTest, ClosesStdoutPipeWhenStderrPipeFails) {
    expectLaunchFailure({2, false}, 2, 0, 2, "Failed to create stderr pipe: ");
}

TEST(ExecLaunchTest, ClosesBothPipesWhenForkFails) {
    expectLaunchFailure({0, true}, 2, 1, 4, "Failed to fork process: ");
}
