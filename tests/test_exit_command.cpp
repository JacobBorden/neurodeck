#include "gtest/gtest.h"
#include "commands/exit.hpp" // Corrected include path
#include "command.hpp"       // For Neurodeck::Command
#include <vector>
#include <string>
#include <memory>
#include <iostream> // For std::cout, std::streambuf (if capturing output)
#include <sstream>  // For std::stringstream

// Test fixture for ExitCommand tests
class ExitCommandTest : public ::testing::Test {
protected:
    Neurodeck::ExitCommand exit_command_; // Instantiate the actual command
    std::stringstream captured_output_;
    std::streambuf* original_cout_buf_;

    void SetUp() override {
        original_cout_buf_ = std::cout.rdbuf(); 
        std::cout.rdbuf(captured_output_.rdbuf()); 
    }

    void TearDown() override {
        std::cout.rdbuf(original_cout_buf_); 
    }
};

TEST_F(ExitCommandTest, NameIsCorrect) {
    EXPECT_EQ(exit_command_.name(), "exit");
}

TEST_F(ExitCommandTest, DescriptionIsNotEmpty) {
    EXPECT_FALSE(exit_command_.description().empty());
    EXPECT_EQ(exit_command_.description(), "Exits the Neurodeck shell.");
}

TEST_F(ExitCommandTest, RunExecutesSuccessfullyAndPrintsMessage) {
    std::vector<std::string> no_args = {"exit"}; // Command name itself as arg[0]
    // The run method itself doesn't throw or cause crashes.
    // The main loop handles the actual exit based on command name.
    EXPECT_NO_THROW(exit_command_.run(no_args));
    
    // Check if it prints the "Exiting..." message
    EXPECT_EQ(captured_output_.str(), "Exiting Neurodeck shell...\n");

    // Test with additional arguments (should be ignored by ExitCommand)
    captured_output_.str(""); // Clear buffer
    captured_output_.clear(); // Clear error flags
    std::vector<std::string> some_args = {"exit", "some", "args"};
    EXPECT_NO_THROW(exit_command_.run(some_args));
    EXPECT_EQ(captured_output_.str(), "Exiting Neurodeck shell...\n");
}
