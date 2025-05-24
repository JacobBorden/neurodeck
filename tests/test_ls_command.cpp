#include "gtest/gtest.h"
#include "commands/ls.hpp" // Corrected include path assuming tests/ is sibling to shell/
#include "command.hpp"       // For Neurodeck::Command
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <memory>

// Test fixture for LsCommand tests
class LsCommandTest : public ::testing::Test {
protected:
    Neurodeck::LsCommand ls_command_; // Instantiate the actual command
    std::stringstream captured_output_;
    std::streambuf* original_cout_buf_;
    std::string expected_ls_output_;

    void SetUp() override {
        original_cout_buf_ = std::cout.rdbuf(); 
        std::cout.rdbuf(captured_output_.rdbuf()); 
        // Updated expected output to match LsCommand's current placeholder output
        expected_ls_output_ = "Available modules/apps (placeholder for ls command):\n"
                              " notes      - textual note manager\n"
                              " calendar   - calendar and event manager\n"
                              " ide        - integrated development environment\n"
                              " calculator - simple calculator\n";
    }

    void TearDown() override {
        std::cout.rdbuf(original_cout_buf_); 
    }
};

TEST_F(LsCommandTest, NameIsCorrect) {
    EXPECT_EQ(ls_command_.name(), "ls");
}

TEST_F(LsCommandTest, DescriptionIsNotEmpty) {
    // Check that the command has a description, as required by the Command interface.
    EXPECT_FALSE(ls_command_.description().empty());
    // Optionally, check the specific description if it's stable:
    EXPECT_EQ(ls_command_.description(), "Lists available modules/apps (placeholder behavior).");
}

TEST_F(LsCommandTest, RunOutputsModuleList) {
    ls_command_.run({}); // Pass an empty vector for args
    std::string output = captured_output_.str();
    EXPECT_EQ(output, expected_ls_output_);
}

TEST_F(LsCommandTest, RunIgnoresArguments) {
    ls_command_.run({"some", "args"}); // Pass some arguments
    std::string output = captured_output_.str();
    // The output should be the same regardless of arguments for this specific command
    EXPECT_EQ(output, expected_ls_output_);
}
