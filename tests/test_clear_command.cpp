#include "gtest/gtest.h"
#include "shell/commands/clear.hpp" // Include the header for ClearCommand
#include <sstream> // For std::stringstream
#include <iostream> // For std::cout, std::streambuf
#include <vector>   // For std::vector
#include <string>   // For std::string
#include <memory>   // For std::unique_ptr

// Test fixture for ClearCommand tests
class ClearCommandTest : public ::testing::Test {
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
};

TEST_F(ClearCommandTest, NameIsCorrect) {
    std::unique_ptr<Command> cmd = make_clear();
    ASSERT_NE(cmd, nullptr);
    EXPECT_EQ(cmd->name(), "clear");
}

TEST_F(ClearCommandTest, RunOutputsCorrectSequence) {
    std::unique_ptr<Command> cmd = make_clear();
    ASSERT_NE(cmd, nullptr);
    
    cmd->run({}); // Pass an empty vector for args
    
    std::string output = captured_output_.str();
    // Note: The problem description has a newline after "Screen cleared."
    // The implementation in clear.cpp also adds a newline.
    EXPECT_EQ(output, "\033[2J\033[1;1HScreen cleared.\n");
}

TEST_F(ClearCommandTest, RunIgnoresArguments) {
    std::unique_ptr<Command> cmd = make_clear();
    ASSERT_NE(cmd, nullptr);
    
    cmd->run({"arg1", "arg2"}); // Pass some arguments
    
    std::string output = captured_output_.str();
    EXPECT_EQ(output, "\033[2J\033[1;1HScreen cleared.\n");
}
