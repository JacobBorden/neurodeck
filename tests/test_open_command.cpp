#include "gtest/gtest.h"
#include "commands/open.hpp" // Corrected include path
#include "command.hpp"       // For Neurodeck::Command
#include <vector>
#include <string>
#include <memory>
#include <iostream> // For std::cout, std::streambuf
#include <sstream>  // For std::stringstream

// Test fixture for OpenCommand tests
class OpenCommandTest : public ::testing::Test {
protected:
    Neurodeck::OpenCommand open_command_; // Instantiate the actual command
    std::stringstream captured_output_;
    std::streambuf* original_cout_buf_;
    std::streambuf* original_cerr_buf_; // Capture cerr for usage message

    void SetUp() override {
        original_cout_buf_ = std::cout.rdbuf(); 
        std::cout.rdbuf(captured_output_.rdbuf()); 
        original_cerr_buf_ = std::cerr.rdbuf();
        std::cerr.rdbuf(captured_output_.rdbuf()); // Redirect cerr to the same stream for simplicity
    }

    void TearDown() override {
        std::cout.rdbuf(original_cout_buf_); 
        std::cerr.rdbuf(original_cerr_buf_);
    }
};

TEST_F(OpenCommandTest, NameIsCorrect) {
    EXPECT_EQ(open_command_.name(), "open");
}

TEST_F(OpenCommandTest, DescriptionIsNotEmpty) {
    EXPECT_FALSE(open_command_.description().empty());
    EXPECT_EQ(open_command_.description(), "Opens a specified file or resource (placeholder). Usage: open <resource_name>");
}

TEST_F(OpenCommandTest, RunExecutesSuccessfullyAndPrintsMessage) {
    // Test with a filename
    open_command_.run({"open", "test_file.txt"}); // Command name as arg[0]
    EXPECT_EQ(captured_output_.str(), "Attempting to open 'test_file.txt' (stub implementation).\n");
    
    // Clear buffer for next test case
    captured_output_.str("");
    captured_output_.clear();

    // Test with no arguments (should print usage to cerr, which is redirected)
    open_command_.run({"open"});
    EXPECT_EQ(captured_output_.str(), "Usage: open <resource_name>\n");

    // Clear buffer for next test case
    captured_output_.str("");
    captured_output_.clear();

    // Test with extra arguments (current stub implementation only uses the first one)
    open_command_.run({"open", "file.txt", "another_arg"});
    EXPECT_EQ(captured_output_.str(), "Attempting to open 'file.txt' (stub implementation).\n");
}
