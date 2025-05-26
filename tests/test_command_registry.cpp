#include "gtest/gtest.h"
#include "command.hpp"         // For Neurodeck::Command base class
#include "command_registry.hpp" // For Neurodeck::CommandRegistry
#include <string>
#include <vector>
#include <memory>      // For std::unique_ptr
#include <algorithm>   // For std::sort, std::find

namespace Neurodeck { // Encapsulate test utilities in the same namespace or a sub-namespace

// 1. StubCommand Implementation (now part of Neurodeck namespace)
class StubCommand : public Neurodeck::Command {
public:
    explicit StubCommand(std::string cmd_name, std::string cmd_desc = "A stub command.") 
        : name_(std::move(cmd_name)), description_(std::move(cmd_desc)), ran_(false) {}

    std::string name() const override { return name_; }
    std::string description() const override { return description_; } // Implemented
    void run(const std::vector<std::string>& args) override { 
        ran_ = true; 
        last_args_ = args; 
    }

    bool ran_ = false; 
    std::vector<std::string> last_args_; 

private:
    std::string name_;
    std::string description_;
};

} // namespace Neurodeck

// Mock factory functions are no longer needed as we directly use CommandRegistry.

// Test Fixture for CommandRegistry tests
class CommandRegistryTest : public ::testing::Test {
protected:
    Neurodeck::CommandRegistry registry_; // Each test gets a fresh registry

    // You can add setup/teardown if needed
    void SetUp() override {
        // Potentially populate with some common stubs or clear state
    }
};

TEST_F(CommandRegistryTest, RegisterAndGetCommand) {
    auto cmd1 = std::make_unique<Neurodeck::StubCommand>("test_cmd_1", "Description 1");
    Neurodeck::Command* cmd1_ptr = cmd1.get(); // Keep raw pointer for comparison if needed before move
    
    registry_.register_command(std::move(cmd1));
    
    Neurodeck::Command* retrieved_cmd = registry_.get_command("test_cmd_1");
    ASSERT_NE(retrieved_cmd, nullptr);
    EXPECT_EQ(retrieved_cmd->name(), "test_cmd_1");
    EXPECT_EQ(retrieved_cmd->description(), "Description 1");
    EXPECT_EQ(retrieved_cmd, cmd1_ptr); // Should be the same object

    // Test getting a non-existent command
    EXPECT_EQ(registry_.get_command("non_existent_cmd"), nullptr);
}

TEST_F(CommandRegistryTest, RegisterDuplicateCommand) {
    registry_.register_command(std::make_unique<Neurodeck::StubCommand>("dup_cmd"));
    // Attempting to register another command with the same name should be handled gracefully
    // (e.g., logged error, original command preserved). The exact behavior depends on
    // CommandRegistry::register_command implementation. Here, we check if original is preserved.
    Neurodeck::Command* original_cmd = registry_.get_command("dup_cmd");
    ASSERT_NE(original_cmd, nullptr);

    // Create a new command with the same name but different description to distinguish
    auto new_cmd_with_same_name = std::make_unique<Neurodeck::StubCommand>("dup_cmd", "New Description");
    registry_.register_command(std::move(new_cmd_with_same_name));

    Neurodeck::Command* current_cmd = registry_.get_command("dup_cmd");
    ASSERT_NE(current_cmd, nullptr);
    // Assuming the registry prevents overwriting, the description should be the original one.
    // Or, if it allows overwriting, it should be "New Description".
    // The current register_command logs an error and does not overwrite.
    EXPECT_EQ(current_cmd->description(), "A stub command."); 
    EXPECT_EQ(current_cmd, original_cmd); // Pointer should be the same as original
}

TEST_F(CommandRegistryTest, UnregisterCommand) {
    registry_.register_command(std::make_unique<Neurodeck::StubCommand>("cmd_to_unregister"));
    ASSERT_NE(registry_.get_command("cmd_to_unregister"), nullptr);

    registry_.unregister_command("cmd_to_unregister");
    EXPECT_EQ(registry_.get_command("cmd_to_unregister"), nullptr);

    // Test unregistering a non-existent command (should not throw)
    EXPECT_NO_THROW(registry_.unregister_command("non_existent_cmd_for_unregister"));
}

TEST_F(CommandRegistryTest, GetAllCommandNames) {
    registry_.register_command(std::make_unique<Neurodeck::StubCommand>("cmd_a"));
    registry_.register_command(std::make_unique<Neurodeck::StubCommand>("cmd_b"));
    registry_.register_command(std::make_unique<Neurodeck::StubCommand>("cmd_c"));

    std::vector<std::string> names = registry_.get_all_command_names();
    ASSERT_EQ(names.size(), 3);
    std::sort(names.begin(), names.end()); // Sort for consistent comparison

    EXPECT_EQ(names[0], "cmd_a");
    EXPECT_EQ(names[1], "cmd_b");
    EXPECT_EQ(names[2], "cmd_c");
}

TEST_F(CommandRegistryTest, PopulateDefaultCommands) {
    // This test now directly uses the real populate_default_commands
    // and checks for the presence of actual default commands.
    Neurodeck::populate_default_commands(registry_);

    // Expected built-in commands (names only, as specific types are not StubCommand)
    const std::vector<std::string> expected_builtins = {
        "ls", "clear", "help", "exit", "open", "loadplugin", "unloadplugin"
    };
    
    EXPECT_GE(registry_.get_all_command_names().size(), expected_builtins.size());

    for (const auto& cmd_name : expected_builtins) {
        Neurodeck::Command* cmd = registry_.get_command(cmd_name);
        ASSERT_NE(cmd, nullptr) << "Built-in command '" << cmd_name << "' not found.";
        EXPECT_EQ(cmd->name(), cmd_name);
        EXPECT_FALSE(cmd->description().empty()) << "Command '" << cmd_name << "' should have a description.";
    }
}

TEST_F(CommandRegistryTest, CommandExecutionViaRegistry) {
    // This test checks if a command retrieved from the registry can be run.
    auto stub_cmd_instance = std::make_unique<Neurodeck::StubCommand>("run_test_cmd");
    
    registry_.register_command(std::move(stub_cmd_instance));
    
    Neurodeck::Command* retrieved_cmd = registry_.get_command("run_test_cmd");
    ASSERT_NE(retrieved_cmd, nullptr);
    
    // Cast back to StubCommand to check its 'ran_' flag.
    // This assumes get_command returns the same type or a base that can be cast.
    Neurodeck::StubCommand* casted_stub_cmd = dynamic_cast<Neurodeck::StubCommand*>(retrieved_cmd);
    ASSERT_NE(casted_stub_cmd, nullptr); // Ensure it's actually our StubCommand type

    EXPECT_FALSE(casted_stub_cmd->ran_) << "Command should not have run yet.";
    std::vector<std::string> test_args = {"run_test_cmd", "arg1"};
    retrieved_cmd->run(test_args); // Run the command
    EXPECT_TRUE(casted_stub_cmd->ran_) << "Command 'ran_' flag not set after execution.";
    EXPECT_EQ(casted_stub_cmd->last_args_, test_args);
}
