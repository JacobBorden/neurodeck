#include "gtest/gtest.h"
#include "shell/commands/load_plugin_command.hpp" // Adjust path as necessary
#include "shell/command_registry.hpp" // Adjust path
#include <sstream> // For capturing cout/cerr
#include <iostream> // For cout/cerr

// A simple mock or fake CommandRegistry for testing purposes
class FakeCommandRegistry : public Neurodeck::CommandRegistry {
public:
    bool load_plugin_called = false;
    std::string loaded_plugin_path;
    bool load_plugin_return_value = true; // Default to success

    FakeCommandRegistry() : Neurodeck::CommandRegistry(nullptr) {} // Base constructor needs a PluginManager pointer

    bool load_plugin(const std::string& path) override {
        load_plugin_called = true;
        loaded_plugin_path = path;
        if (load_plugin_return_value) {
            std::cout << "FakeRegistry: Plugin '" << path << "' loaded." << std::endl;
        } else {
            std::cerr << "FakeRegistry: Failed to load plugin '" << path << "'." << std::endl;
        }
        return load_plugin_return_value;
    }

    // Implement other pure virtual methods if any, or ensure base class handles them if not abstract
    // For this test, we only care about load_plugin.
    // Add minimal implementations for other virtual methods from CommandRegistry if they are pure virtual
     void register_command(std::shared_ptr<Neurodeck::Command>) override {}
     void unregister_command(const std::string&) override {}
     std::shared_ptr<Neurodeck::Command> get_command(const std::string&) const override { return nullptr;}
     std::map<std::string, std::shared_ptr<Neurodeck::Command>> get_commands() const override { return {}; }
     bool unload_plugin(const std::string& path) override { return true; } // Basic stub
     void set_plugin_manager(Neurodeck::PluginManager* manager) override {} // Basic stub

};

TEST(LoadPluginCommandTest, NameAndDescription) {
    FakeCommandRegistry fake_registry;
    Neurodeck::LoadPluginCommand cmd(fake_registry);
    ASSERT_EQ(cmd.name(), "loadplugin");
    ASSERT_FALSE(cmd.description().empty());
}

TEST(LoadPluginCommandTest, RunSuccess) {
    FakeCommandRegistry fake_registry;
    Neurodeck::LoadPluginCommand cmd(fake_registry);
    std::vector<std::string> args = {"loadplugin", "test_plugin.dll"};

    // Capture cout
    std::streambuf* old_cout = std::cout.rdbuf();
    std::ostringstream captured_cout;
    std::cout.rdbuf(captured_cout.rdbuf());

    cmd.run(args);

    std::cout.rdbuf(old_cout); // Restore cout

    ASSERT_TRUE(fake_registry.load_plugin_called);
    ASSERT_EQ(fake_registry.loaded_plugin_path, "test_plugin.dll");
    // Check for success message (from FakeRegistry, as command itself is silent on success)
    ASSERT_NE(captured_cout.str().find("FakeRegistry: Plugin 'test_plugin.dll' loaded."), std::string::npos);
}

TEST(LoadPluginCommandTest, RunFailure) {
    FakeCommandRegistry fake_registry;
    fake_registry.load_plugin_return_value = false; // Simulate failure
    Neurodeck::LoadPluginCommand cmd(fake_registry);
    std::vector<std::string> args = {"loadplugin", "test_plugin_fail.dll"};

    // Capture cerr
    std::streambuf* old_cerr = std::cerr.rdbuf();
    std::ostringstream captured_cerr;
    std::cerr.rdbuf(captured_cerr.rdbuf());

    cmd.run(args);

    std::cerr.rdbuf(old_cerr); // Restore cerr

    ASSERT_TRUE(fake_registry.load_plugin_called);
    ASSERT_EQ(fake_registry.loaded_plugin_path, "test_plugin_fail.dll");
    // Check for failure message (from FakeRegistry)
    ASSERT_NE(captured_cerr.str().find("FakeRegistry: Failed to load plugin 'test_plugin_fail.dll'."), std::string::npos);
}

TEST(LoadPluginCommandTest, NotEnoughArguments) {
    FakeCommandRegistry fake_registry;
    Neurodeck::LoadPluginCommand cmd(fake_registry);
    std::vector<std::string> args = {"loadplugin"}; // Missing path

    // Capture cerr
    std::streambuf* old_cerr = std::cerr.rdbuf();
    std::ostringstream captured_cerr;
    std::cerr.rdbuf(captured_cerr.rdbuf());

    cmd.run(args);

    std::cerr.rdbuf(old_cerr); // Restore cerr

    ASSERT_FALSE(fake_registry.load_plugin_called); // Should not be called
    ASSERT_NE(captured_cerr.str().find("Usage: loadplugin <path_to_plugin_file>"), std::string::npos);
}
