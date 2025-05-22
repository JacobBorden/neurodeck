#include "gtest/gtest.h"
#include "../shell/commands/ls.hpp" // Include the header for LsCommand
#include <sstream> // For std::stringstream
#include <iostream> // For std::cout, std::streambuf
#include <vector>   // For std::vector
#include <string>   // For std::string
#include <memory>   // For std::unique_ptr

// Test fixture for LsCommand tests
class LsCommandTest : public ::testing::Test {
protected:
    std::stringstream captured_output_;
    std::streambuf* original_cout_buf_;
    std::string expected_ls_output_;

    void SetUp() override {
        original_cout_buf_ = std::cout.rdbuf(); // Save original cout buffer
        std::cout.rdbuf(captured_output_.rdbuf()); // Redirect cout to stringstream
        expected_ls_output_ = "Available modules/apps:\n"
                              " notes  - textual note manager\n"
                              " calendar - calendar and event manager\n"
                              " ide - integrated development environment\n"
                              " calculator - simple calculator\n";
    }

    void TearDown() override {
        std::cout.rdbuf(original_cout_buf_); // Restore original cout buffer
    }
};

TEST_F(LsCommandTest, NameIsCorrect) {
    std::unique_ptr<Command> cmd = make_ls();
    ASSERT_NE(cmd, nullptr);
    EXPECT_EQ(cmd->name(), "ls");
}

TEST_F(LsCommandTest, RunOutputsModuleList) {
    std::unique_ptr<Command> cmd = make_ls();
    ASSERT_NE(cmd, nullptr);
    
    cmd->run({}); // Pass an empty vector for args
    
    std::string output = captured_output_.str();
    EXPECT_EQ(output, expected_ls_output_);
}

TEST_F(LsCommandTest, RunIgnoresArguments) {
    std::unique_ptr<Command> cmd = make_ls();
    ASSERT_NE(cmd, nullptr);
    
    cmd->run({"some", "args"}); // Pass some arguments
    
    std::string output = captured_output_.str();
    EXPECT_EQ(output, expected_ls_output_);
}
