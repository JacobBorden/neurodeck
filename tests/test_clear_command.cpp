#include "gtest/gtest.h"
#include "commands/clear.hpp" // Corrected include path
#include "command.hpp"        // For Neurodeck::Command
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <memory>

// Test fixture for ClearCommand tests
class ClearCommandTest : public ::testing::Test {
protected:
    Neurodeck::ClearCommand clear_command_; // Instantiate the actual command
    std::stringstream captured_output_;
    std::streambuf* original_cout_buf_;
    std::string expected_clear_sequence_ = "\033[2J\033[1;1H"; 
    // Note: The ClearCommand itself was modified not to print "Screen cleared.\n"
    // as it might be wiped too quickly or be undesirable. Testing only for sequence.

    void SetUp() override {
        original_cout_buf_ = std::cout.rdbuf(); 
        std::cout.rdbuf(captured_output_.rdbuf()); 
    }

    void TearDown() override {
        std::cout.rdbuf(original_cout_buf_); 
    }
};

TEST_F(ClearCommandTest, NameIsCorrect) {
    EXPECT_EQ(clear_command_.name(), "clear");
}

TEST_F(ClearCommandTest, DescriptionIsNotEmpty) {
    EXPECT_FALSE(clear_command_.description().empty());
    EXPECT_EQ(clear_command_.description(), "Clears the terminal screen.");
}

TEST_F(ClearCommandTest, RunOutputsCorrectSequence) {
    clear_command_.run({}); // Pass an empty vector for args
    std::string output = captured_output_.str();
    EXPECT_EQ(output, expected_clear_sequence_);
}

TEST_F(ClearCommandTest, RunIgnoresArguments) {
    clear_command_.run({"arg1", "arg2"}); // Pass some arguments
    std::string output = captured_output_.str();
    EXPECT_EQ(output, expected_clear_sequence_);
}
