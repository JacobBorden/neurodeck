#include "gtest/gtest.h"
#include "shell/commands/exec_command.hpp" // Adjust path as necessary
#include "shell/command_registry.hpp" // For CommandRegistry, if needed for context, though exec_command can be tested directly
#include <vector>
#include <string>
#include <iostream>
#include <sstream> // Required for std::stringstream

// Helper function to simulate running the command and capture output
// This is a simplified approach. In a real scenario, you might need to
// temporarily redirect std::cout and std::cerr to stringstreams.
// For ExecCommand, its run method already prints to std::cout and std::cerr.
// We need to capture that.

class ExecCommandTest : public ::testing::Test {
protected:
    Neurodeck::ExecCommand exec_command;
    std::vector<std::string> args;
    std::stringstream captured_cout;
    std::stringstream captured_cerr;
    std::streambuf* original_cout_rdbuf;
    std::streambuf* original_cerr_rdbuf;

    void SetUp() override {
        // Save original buffer settings
        original_cout_rdbuf = std::cout.rdbuf();
        original_cerr_rdbuf = std::cerr.rdbuf();
        // Redirect std::cout and std::cerr to our stringstreams
        std::cout.rdbuf(captured_cout.rdbuf());
        std::cerr.rdbuf(captured_cerr.rdbuf());
    }

    void TearDown() override {
        // Restore original buffer settings
        std::cout.rdbuf(original_cout_rdbuf);
        std::cerr.rdbuf(original_cerr_rdbuf);
    }

    // Helper to run the command and get combined output for easier testing in some cases
    // For specific stdout/stderr checks, use captured_cout and captured_cerr directly.
    void runCommand(const std::vector<std::string>& cmd_args) {
        args = cmd_args; // Corrected: use the passed cmd_args to set member variable
        exec_command.run(args); // Corrected: pass the member variable 'args'
    }
};

TEST_F(ExecCommandTest, EchoCommand) {
    runCommand({"exec", "echo", "Hello", "World"});
    std::string cout_output = captured_cout.str();
    std::string cerr_output = captured_cerr.str();
    
    // Expect "Stdout:
Hello World
"
    // The "Stdout:
" prefix and trailing newline are from ExecCommand's formatting
    // The actual echo command adds a newline too.
    EXPECT_NE(cout_output.find("Hello World"), std::string::npos);
    EXPECT_TRUE(cerr_output.find("Stderr:") == std::string::npos || captured_cerr.str().find("Command exited with status") != std::string::npos); // Allow exit status message
}

TEST_F(ExecCommandTest, LsCommandExists) {
    // This test assumes 'ls' exists and /tmp is readable.
    // It's a bit environment-dependent but common.
    runCommand({"exec", "ls", "/"}); // List root directory
    std::string cout_output = captured_cout.str();
    EXPECT_NE(cout_output.find("Stdout:"), std::string::npos); // Check if Stdout: prefix is present
    // We can't check for specific files easily across systems, so just check it ran.
    // A more robust test would be to use a command that behaves identically everywhere, like `echo`.
}


TEST_F(ExecCommandTest, CommandNotFound) {
    runCommand({"exec", "a_very_unlikely_command_to_exist_12345"});
    std::string cerr_output = captured_cerr.str();
    
    // The error message from execvp (via child's stderr) should be captured.
    // It might also include "Command exited with status" if execvp failure leads to non-zero exit.
    // Example: "Stderr:
Failed to execute command '...': No such file or directory
Command exited with status 1
"
    // Or, if child directly _exit(EXIT_FAILURE) after perror for execvp:
    // "Stderr:
Failed to execute command 'a_very_unlikely_command_to_exist_12345': No such file or directory
"
    // "Command exited with status 127" (common for command not found) or "Command exited with status 1" (if _exit(EXIT_FAILURE))
    EXPECT_TRUE(cerr_output.find("Failed to execute command") != std::string::npos || cerr_output.find("No such file or directory") != std::string::npos);
    EXPECT_TRUE(cerr_output.find("Command exited with status") != std::string::npos);
}

TEST_F(ExecCommandTest, CommandWritesToStdErr) {
    // Use a command that is likely to write to stderr.
    // e.g. 'ls' a non-existent file.
    // Note: shell error message format can vary.
    runCommand({"exec", "ls", "/non_existent_path_for_testing_123"});
    std::string cerr_output = captured_cerr.str();

    // Expect something like:
    // Stderr:
    // ls: cannot access '/non_existent_path_for_testing_123': No such file or directory
    // Command exited with status X 
    // (X is often 1 or 2 for ls errors)
    EXPECT_NE(cerr_output.find("Stderr:"), std::string::npos);
    EXPECT_TRUE(cerr_output.find("No such file or directory") != std::string::npos || cerr_output.find("cannot access") != std::string::npos); // Common messages
    EXPECT_TRUE(cerr_output.find("Command exited with status") != std::string::npos);
}

TEST_F(ExecCommandTest, NoArgumentsToExec) {
    runCommand({"exec"}); // Only "exec" itself
    std::string cerr_output = captured_cerr.str();
    // ExecCommand::run should print usage to std::cerr directly
    EXPECT_NE(cerr_output.find("Usage: exec <command> [args...]"), std::string::npos);
}

TEST_F(ExecCommandTest, EchoToStderrViaShellRedirection) {
    // This tests if the spawned shell can itself redirect.
    // This is more of an advanced test of the capabilities of the command being exec'd.
    // We'll use sh -c 'command >&2' to force output to stderr.
    runCommand({"exec", "sh", "-c", "echo 'Error message via sh' >&2"});
    std::string cout_output = captured_cout.str();
    std::string cerr_output = captured_cerr.str();

    EXPECT_TRUE(cout_output.find("Error message via sh") == std::string::npos); // Should not be in stdout
    EXPECT_NE(cerr_output.find("Error message via sh"), std::string::npos);
    // It might also include "Command exited with status 0" if sh ran successfully.
}

// It might be useful to add a test for commands with many arguments,
// or arguments containing spaces (though tokenization is handled before ExecCommand).
// However, ExecCommand itself just passes them through.

// A test for PATH searching (e.g. exec "ls" vs exec "/bin/ls") could be valuable
// but execvp already handles PATH, so we are testing the OS behavior more than our code.
// The current "ls" test implicitly checks PATH searching if "ls" is not /bin/ls on test system.
// Removed spurious closing brace from original prompt
