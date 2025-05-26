#include "gtest/gtest.h"
#include "commands/unload_plugin_command.hpp" // Adjust path
#include "command_registry.hpp" // Adjust path
#include <sstream>
#include <iostream>

// Re-use or adapt FakeCommandRegistry or create a new one for unload
class FakeCommandRegistryForUnload : public Neurodeck::CommandRegistry {
public:
    bool unload_plugin_called = false;
    std::string unloaded_plugin_path;
    bool unload_plugin_return_value = true;

    FakeCommandRegistryForUnload() : Neurodeck::CommandRegistry() {}

    // This hides the base class method for testing
    bool unload_plugin(const std::string& path) {
        unload_plugin_called = true;
        unloaded_plugin_path = path;
        if (unload_plugin_return_value) {
            std::cout << "FakeRegistry: Plugin '" << path << "' unloaded." << std::endl;
        } else {
            std::cerr << "FakeRegistry: Failed to unload plugin '" << path << "'." << std::endl;
        }
        return unload_plugin_return_value;
    }
};


TEST(UnloadPluginCommandTest, NameAndDescription) {
    FakeCommandRegistryForUnload fake_registry;
    Neurodeck::UnloadPluginCommand cmd(fake_registry);
    ASSERT_EQ(cmd.name(), "unloadplugin");
    ASSERT_FALSE(cmd.description().empty());
}

TEST(UnloadPluginCommandTest, RunSuccess) {
    FakeCommandRegistryForUnload fake_registry;
    Neurodeck::UnloadPluginCommand cmd(fake_registry);
    std::vector<std::string> args = {"unloadplugin", "test_plugin.dll"};

    std::streambuf* old_cout = std::cout.rdbuf();
    std::ostringstream captured_cout;
    std::cout.rdbuf(captured_cout.rdbuf());

    cmd.run(args);

    std::cout.rdbuf(old_cout);

    ASSERT_TRUE(fake_registry.unload_plugin_called);
    ASSERT_EQ(fake_registry.unloaded_plugin_path, "test_plugin.dll");
}

TEST(UnloadPluginCommandTest, RunFailure) {
    FakeCommandRegistryForUnload fake_registry;
    fake_registry.unload_plugin_return_value = false; // Simulate failure
    Neurodeck::UnloadPluginCommand cmd(fake_registry);
    std::vector<std::string> args = {"unloadplugin", "test_plugin_fail.dll"};
    
    std::streambuf* old_cerr = std::cerr.rdbuf();
    std::ostringstream captured_cerr;
    std::cerr.rdbuf(captured_cerr.rdbuf());

    cmd.run(args);

    std::cerr.rdbuf(old_cerr);
    
    ASSERT_TRUE(fake_registry.unload_plugin_called);
    ASSERT_EQ(fake_registry.unloaded_plugin_path, "test_plugin_fail.dll");
}


TEST(UnloadPluginCommandTest, NotEnoughArguments) {
    FakeCommandRegistryForUnload fake_registry;
    Neurodeck::UnloadPluginCommand cmd(fake_registry);
    std::vector<std::string> args = {"unloadplugin"};

    std::streambuf* old_cerr = std::cerr.rdbuf();
    std::ostringstream captured_cerr;
    std::cerr.rdbuf(captured_cerr.rdbuf());

    cmd.run(args);

    std::cerr.rdbuf(old_cerr);

    ASSERT_FALSE(fake_registry.unload_plugin_called);
    ASSERT_NE(captured_cerr.str().find("Usage: unloadplugin <path_to_plugin_file>"), std::string::npos);
}
