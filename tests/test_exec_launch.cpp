#include "commands/exec_command.hpp"
#include <gtest/gtest.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
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

struct PollFailure {
    int error;
    int calls = 0;
    std::array<int, 2> read_fds{-1, -1};
};

PollFailure* active_poll_failure = nullptr;

class InjectPollFailure {
public:
    explicit InjectPollFailure(PollFailure& failure) { active_poll_failure = &failure; }
    InjectPollFailure(const InjectPollFailure&) = delete;
    InjectPollFailure& operator=(const InjectPollFailure&) = delete;
    ~InjectPollFailure() { active_poll_failure = nullptr; }
};

enum class ReadFault { Interrupted, ErrorWithData, ErrorInsteadOfEof };

struct ReadFailure {
    const LaunchFailure& launch;
    ReadFault fault;
    int fd = -1;
    int calls = 0;
    bool injected = false;
};

ReadFailure* active_read_failure = nullptr;

class InjectReadFailure {
public:
    explicit InjectReadFailure(ReadFailure& failure) { active_read_failure = &failure; }
    InjectReadFailure(const InjectReadFailure&) = delete;
    InjectReadFailure& operator=(const InjectReadFailure&) = delete;
    ~InjectReadFailure() { active_read_failure = nullptr; }
};

} // namespace

extern "C" int __real_pipe(int* fds);
extern "C" int __real_poll(pollfd* fds, nfds_t count, int timeout);
extern "C" ssize_t __real_read(int fd, void* buffer, size_t count);
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

extern "C" int __wrap_poll(pollfd* fds, nfds_t count, int timeout) {
    auto* failure = active_poll_failure;
    if (failure && count == failure->read_fds.size() && ++failure->calls == 1) {
        failure->read_fds = {fds[0].fd, fds[1].fd};
        errno = failure->error;
        return -1;
    }
    return __real_poll(fds, count, timeout);
}

extern "C" ssize_t __wrap_read(int fd, void* buffer, size_t count) {
    auto* failure = active_read_failure;
    // The second pipe captures stderr. Leave stdout and unrelated reads alone.
    if (!failure || failure->launch.opened_count != 4 || fd != failure->launch.opened_fds[2]) {
        return __real_read(fd, buffer, count);
    }

    failure->fd = fd;
    ++failure->calls;
    if (failure->fault == ReadFault::Interrupted && !failure->injected) {
        failure->injected = true;
        errno = EINTR;
        return -1;
    }

    const ssize_t result = __real_read(fd, buffer, count);
    // The fixture writes stderr once. Consume those bytes before injecting EIO
    // so closing the read end cannot race that write and cause SIGPIPE.
    const bool error_with_data = failure->fault == ReadFault::ErrorWithData && result > 0;
    // Replacing EOF with EIO leaves previously delivered bytes in the capture.
    const bool error_instead_of_eof = failure->fault == ReadFault::ErrorInsteadOfEof && result == 0;
    if (!failure->injected && (error_with_data || error_instead_of_eof)) {
        failure->injected = true;
        errno = EIO;
        return -1;
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

TEST(ExecPollTest, RetriesInterruptedPollAndPreservesBothStreams) {
    PollFailure failure{EINTR};
    Neurodeck::ExecCommand command;
    CaptureStream output(std::cout);
    CaptureStream errors(std::cerr);
    {
        InjectPollFailure inject(failure);
        command.run({"exec", "/bin/sh", "-c", "printf out; printf err >&2"});
    }
    EXPECT_GT(failure.calls, 1);
    EXPECT_EQ(output.str(), "Stdout:\nout\n");
    EXPECT_EQ(errors.str(), "Stderr:\nerr\n");
}

TEST(ExecPollTest, ClosesBothReadEndsAndReapsChildAfterTerminalPollError) {
    PollFailure failure{EIO};
    WaitFailure wait{0};
    Neurodeck::ExecCommand command;
    CaptureStream output(std::cout);
    CaptureStream errors(std::cerr);
    {
        InjectPollFailure inject_poll(failure);
        InjectWaitFailure observe_wait(wait);
        command.run({"exec", "/bin/true"});
    }
    EXPECT_EQ(failure.calls, 1);
    EXPECT_TRUE(output.str().empty());
    EXPECT_EQ(errors.str(), std::string("Error polling child output: ") + std::strerror(EIO) + "\n");
    for (const int fd : failure.read_fds) {
        ASSERT_GE(fd, 0);
        errno = 0;
        const int result = fcntl(fd, F_GETFD);
        const int error = errno;
        EXPECT_EQ(result, -1) << "pipe fd " << fd;
        EXPECT_EQ(error, EBADF) << "pipe fd " << fd;
    }
    EXPECT_NE(failure.read_fds[0], failure.read_fds[1]);
    EXPECT_EQ(wait.calls, 1);
    EXPECT_GT(wait.child, 0);
    EXPECT_EQ(wait.reaped, wait.child);
}

namespace {

void expectReadFailure(ReadFault fault, const std::string& expected_stderr) {
    LaunchFailure launch{0, false};
    ReadFailure read_failure{launch, fault};
    WaitFailure wait{0};
    Neurodeck::ExecCommand command;
    CaptureStream output(std::cout);
    CaptureStream errors(std::cerr);
    {
        InjectLaunchFailure observe_pipes(launch);
        InjectReadFailure inject_read(read_failure);
        InjectWaitFailure observe_wait(wait);
        command.run({"exec", "/bin/sh", "-c", "printf out; printf err >&2"});
    }

    EXPECT_TRUE(read_failure.injected);
    if (fault == ReadFault::ErrorWithData) {
        EXPECT_EQ(read_failure.calls, 1);
    } else {
        EXPECT_GT(read_failure.calls, 1);
    }
    EXPECT_EQ(output.str(), "Stdout:\nout\n");
    EXPECT_EQ(errors.str(), expected_stderr);
    EXPECT_EQ(wait.calls, 1);
    EXPECT_GT(wait.child, 0);
    EXPECT_EQ(wait.reaped, wait.child);
    ASSERT_GE(read_failure.fd, 0);
    errno = 0;
    const int result = fcntl(read_failure.fd, F_GETFD);
    const int fd_error = errno;
    EXPECT_EQ(result, -1);
    EXPECT_EQ(fd_error, EBADF);
}

} // namespace

TEST(ExecReadTest, RetriesInterruptedReadAndPreservesBothStreams) {
    expectReadFailure(ReadFault::Interrupted, "Stderr:\nerr\n");
}

TEST(ExecReadTest, ClosesFailedStreamAndPreservesOtherStream) {
    expectReadFailure(ReadFault::ErrorWithData,
                      std::string("Error reading from child stderr: ") + std::strerror(EIO) + "\n");
}

TEST(ExecReadTest, PreservesPreviouslyCapturedBytesAfterTerminalReadError) {
    expectReadFailure(ReadFault::ErrorInsteadOfEof,
                      std::string("Error reading from child stderr: ") + std::strerror(EIO) +
                          "\nStderr:\nerr\n");
}
