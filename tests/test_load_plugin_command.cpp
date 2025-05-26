#include "gtest/gtest.h"
#include "commands/load_plugin_command.hpp" // Adjust path as necessary
#include "command_registry.hpp" // Adjust path
#include <sstream> // For capturing cout/cerr
#include <iostream> // For cout/cerr

// A simple mock or fake CommandRegistry for testing purposes
class FakeCommandRegistry : public Neurodeck::CommandRegistry {
public:
    bool load_plugin_called = false;
    std::string loaded_plugin_path;
    bool load_plugin_return_value = true; // Default to success

    FakeCommandRegistry() : Neurodeck::CommandRegistry() {}

    // This hides the base class method, but that's fine for the test
    bool load_plugin(const std::string& path) {
        load_plugin_called = true;
        loaded_plugin_path = path;
        if (load_plugin_return_value) {
            std::cout << "FakeRegistry: Plugin '" << path << "' loaded." << std::endl;
        } else {
            std::cerr << "FakeRegistry: Failed to load plugin '" << path << "'." << std::endl;
        }
        return load_plugin_return_value;
    }

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
    // No need to check for FakeRegistry output in captured_cout
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
    // No need to check for FakeRegistry output in captured_cerr
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
