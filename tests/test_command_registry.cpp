#include "gtest/gtest.h"
#include "../shell/command.hpp"         // For Command base class
#include "../shell/command_registry.hpp" // For build_registry() declaration
#include <string>
#include <vector>
#include <memory>      // For std::unique_ptr
#include <unordered_map> // For the registry type

// 1. StubCommand Implementation
class StubCommand : public Command {
public:
    explicit StubCommand(std::string cmd_name) : name_(std::move(cmd_name)), ran_(false) {}

    std::string name() const override { return name_; }
    void run(const std::vector<std::string>& args) override { 
        ran_ = true; 
        // Optionally, store args if needed for more detailed tests
        // last_args_ = args; 
    }

    bool ran_ = false; // Public for easy inspection in tests
    // std::vector<std::string> last_args_; // Example if args needed to be checked

private:
    std::string name_;
};

// 2. Mock Factory Functions
// These functions are declared as 'extern' in shell/command.cpp.
// By defining them here, the linker will use these versions when
// compiling and linking this test file into the runTests executable.
std::unique_ptr<Command> make_ls() { 
    return std::make_unique<StubCommand>("ls"); 
}
std::unique_ptr<Command> make_clear() { 
    return std::make_unique<StubCommand>("clear"); 
}
std::unique_ptr<Command> make_help() { 
    return std::make_unique<StubCommand>("help"); 
}
std::unique_ptr<Command> make_exit() { 
    return std::make_unique<StubCommand>("exit"); 
}
std::unique_ptr<Command> make_open() { 
    return std::make_unique<StubCommand>("open"); 
}

// 3. Test Cases
class CommandRegistryTest : public ::testing::Test {
protected:
    // You can add setup/teardown if needed, but for these tests,
    // it might not be necessary.
};

TEST_F(CommandRegistryTest, BuildRegistryPopulatesCorrectly) {
    auto registry = build_registry();

    // Expected number of commands
    const size_t expected_command_count = 5;
    ASSERT_EQ(registry.size(), expected_command_count) 
        << "Registry does not contain the expected number of commands.";

    // List of expected command names
    const std::vector<std::string> expected_commands = {"ls", "clear", "help", "exit", "open"};

    for (const auto& cmd_name : expected_commands) {
        auto it = registry.find(cmd_name);
        ASSERT_NE(it, registry.end()) << "Command '" << cmd_name << "' not found in registry.";
        ASSERT_NE(it->second, nullptr) << "Command '" << cmd_name << "' pointer is null.";
        
        // Check if the name() method of the command object matches
        EXPECT_EQ(it->second->name(), cmd_name) 
            << "Command '" << cmd_name << "' has incorrect name stored in the Command object.";
        
        // Optional: Cast to StubCommand to check specific stub properties if needed
        // auto* stub_cmd = dynamic_cast<StubCommand*>(it->second.get());
        // ASSERT_NE(stub_cmd, nullptr) << "Command '" << cmd_name << "' is not a StubCommand.";
    }
}

TEST_F(CommandRegistryTest, StubCommandsAreUsed) {
    auto registry = build_registry();
    ASSERT_FALSE(registry.empty());

    for(const auto& pair : registry) {
        ASSERT_NE(pair.second, nullptr);
        // Try to cast to StubCommand. If it fails, it's not our stub.
        StubCommand* stub = dynamic_cast<StubCommand*>(pair.second.get());
        ASSERT_NE(stub, nullptr) << "Command '" << pair.first << "' is not a StubCommand. Mocks might not be linked correctly.";
        
        // Check initial state of 'ran_' flag
        EXPECT_FALSE(stub->ran_) << "StubCommand '" << pair.first << "' should not have ran yet.";
        
        // Call run and check if 'ran_' flag is set
        std::vector<std::string> empty_args;
        stub->run(empty_args);
        EXPECT_TRUE(stub->ran_) << "StubCommand '" << pair.first << "' did not set 'ran_' flag after run().";
    }
}
