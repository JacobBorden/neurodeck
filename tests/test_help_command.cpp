#include "gtest/gtest.h"
#include "../shell/commands/help.hpp" // Include the header for HelpCommand
#include <sstream> // For std::stringstream
#include <iostream> // For std::cout, std::streambuf
#include <vector>   // For std::vector
#include <string>   // For std::string
#include <memory>   // For std::unique_ptr

// Test fixture for HelpCommand tests
class HelpCommandTest : public ::testing::Test {
protected:
    std::stringstream captured_output_;
    std::streambuf* original_cout_buf_;

    void SetUp() override {
        original_cout_buf_ = std::cout.rdbuf(); // Save original cout buffer
        std::cout.rdbuf(captured_output_.rdbuf()); // Redirect cout to stringstream
    }

    void TearDown() override {
        std::cout.rdbuf(original_cout_buf_); // Restore original cout buffer
    }

    // Helper to define the expected help string, matching the implementation
    const std::string expected_help_string_ = 
        "Available commands:\n"
        " ls - List files in the current directory\n"
        " clear - Clear the screen\n"
        " open <filename> - Open a file\n"
        " exit - Exit the shell\n"
        " help - Show this help message\n";
};

TEST_F(HelpCommandTest, NameIsCorrect) {
    std::unique_ptr<Command> cmd = make_help();
    ASSERT_NE(cmd, nullptr);
    EXPECT_EQ(cmd->name(), "help");
}

TEST_F(HelpCommandTest, RunOutputsHelpMessage) {
    std::unique_ptr<Command> cmd = make_help();
    ASSERT_NE(cmd, nullptr);
    
    cmd->run({}); // Pass an empty vector for args
    
    std::string output = captured_output_.str();
    EXPECT_EQ(output, expected_help_string_);
}

TEST_F(HelpCommandTest, RunIgnoresArguments) {
    std::unique_ptr<Command> cmd = make_help();
    ASSERT_NE(cmd, nullptr);
    
    cmd->run({"some", "args"}); // Pass some arguments
    
    std::string output = captured_output_.str();
    EXPECT_EQ(output, expected_help_string_);
}
