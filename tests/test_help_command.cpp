#include "gtest/gtest.h"
#include "commands/help.hpp"    // Corrected include path
#include "command_registry.hpp" // For CommandRegistry
#include "command.hpp"          // For Neurodeck::Command
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm> // For std::sort, std::string::find
#include <iomanip>   // For std::setw

// Test fixture for HelpCommand tests
class HelpCommandTest : public ::testing::Test {
protected:
    Neurodeck::CommandRegistry registry_;
    Neurodeck::HelpCommand help_command_; // Instantiate the actual command with the registry
    std::stringstream captured_output_;
    std::streambuf* original_cout_buf_;

    HelpCommandTest() : help_command_(registry_) {} // Initialize help_command_ with registry_

    void SetUp() override {
        original_cout_buf_ = std::cout.rdbuf(); 
        std::cout.rdbuf(captured_output_.rdbuf()); 
        // Populate the registry with some commands for testing help output
        // Using StubCommand from test_command_registry.cpp (assuming it's accessible or redefined here)
        // For simplicity, we'll use real commands if HelpCommand tests the actual descriptions.
        Neurodeck::populate_default_commands(registry_); // Use actual default commands
    }

    void TearDown() override {
        std::cout.rdbuf(original_cout_buf_); 
    }
};

TEST_F(HelpCommandTest, NameIsCorrect) {
    EXPECT_EQ(help_command_.name(), "help");
}

TEST_F(HelpCommandTest, DescriptionIsNotEmpty) {
    EXPECT_FALSE(help_command_.description().empty());
    EXPECT_EQ(help_command_.description(), "Shows this help message, listing all available commands and their descriptions.");
}

TEST_F(HelpCommandTest, RunOutputsFormattedHelpMessage) {
    help_command_.run({}); // Pass an empty vector for args
    
    std::string output = captured_output_.str();
    
    // Check for key phrases
    EXPECT_NE(output.find("Available commands:"), std::string::npos);
    
    // Check if all default commands are listed and have descriptions
    std::vector<std::string> default_command_names = {
        "help", "exit", "open", "loadplugin", "unloadplugin", "exec", "lua"
    };
    // Sort them as HelpCommand sorts its output
    std::sort(default_command_names.begin(), default_command_names.end());

    size_t max_name_length = 0;
    for (const auto& name : default_command_names) {
        if (name.length() > max_name_length) {
            max_name_length = name.length();
        }
    }
    
    for (const auto& name : default_command_names) {
        Neurodeck::Command* cmd = registry_.get_command(name);
        ASSERT_NE(cmd, nullptr);
        
        // Construct the expected line format (approximately)
        // Example: "  clear       - Clears the terminal screen."
        std::ostringstream expected_line_stream;
        expected_line_stream << "  " << std::left << std::setw(max_name_length + 2) << name 
                             << "- " << cmd->description();
        
        EXPECT_NE(output.find(name), std::string::npos) << "Command name '" << name << "' not found in help output.";
        EXPECT_NE(output.find(cmd->description()), std::string::npos) << "Description for '" << name << "' not found in help output.";
        EXPECT_NE(output.find(expected_line_stream.str()), std::string::npos) 
            << "Formatted line for '" << name << "' not found or incorrect. Expected: \n'" 
            << expected_line_stream.str() << "'\nActual output relevant part might differ slightly due to setw and exact spacing.";
    }
    EXPECT_NE(output.find("Type '<command_name> --help'"), std::string::npos);
}

TEST_F(HelpCommandTest, RunIgnoresArguments) {
    // Clear previous output
    captured_output_.str("");
    captured_output_.clear();

    help_command_.run({"some", "args"}); // Pass some arguments
    
    std::string output_with_args = captured_output_.str();
    
    // Re-run without arguments to compare
    captured_output_.str("");
    captured_output_.clear();
    help_command_.run({});
    std::string output_without_args = captured_output_.str();
    
    // The output should be the same as HelpCommand ignores arguments
    EXPECT_EQ(output_with_args, output_without_args);
}
