#include "gtest/gtest.h"
#include "shell/command_registry.hpp" // For CommandRegistry
#include "shell/command.hpp"          // For Command
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <sstream> // Required for std::stringstream

// Helper function to capture stdout (basic version)
class CoutRedirect {
public:
    CoutRedirect(std::streambuf* new_buffer)
        : old(std::cout.rdbuf(new_buffer)) {}
    ~CoutRedirect() {
        std::cout.rdbuf(old);
    }
private:
    std::streambuf* old;
};

// Test fixture for plugin system tests
class UnifiedPluginSystemTest : public ::testing::Test {
protected:
    Neurodeck::CommandRegistry registry;
    std::stringstream captured_cout;

    std::string cpp_hello_plugin_path = "";
    std::string cpp_echo_plugin_path = "";
    std::string lua_hello_plugin_path = "../../scripts/plugins/hello.lua"; // Default, might be overridden
    std::string lua_echo_plugin_path = "../../scripts/plugins/echo.lua";   // Default, might be overridden

    void SetUp() override {
        const char* cpp_hello_env = std::getenv("TEST_CPP_HELLO_PLUGIN_PATH");
        if (cpp_hello_env) cpp_hello_plugin_path = cpp_hello_env;

        const char* cpp_echo_env = std::getenv("TEST_CPP_ECHO_PLUGIN_PATH");
        if (cpp_echo_env) cpp_echo_plugin_path = cpp_echo_env;

        const char* lua_hello_env = std::getenv("TEST_LUA_HELLO_PLUGIN_PATH");
        if (lua_hello_env) lua_hello_plugin_path = lua_hello_env;
        else if (std::filesystem::exists("scripts/plugins/hello.lua")) { // Adjust if running from project root
            lua_hello_plugin_path = "scripts/plugins/hello.lua";
        }


        const char* lua_echo_env = std::getenv("TEST_LUA_ECHO_PLUGIN_PATH");
        if (lua_echo_env) lua_echo_plugin_path = lua_echo_env;
        else if (std::filesystem::exists("scripts/plugins/echo.lua")) { // Adjust if running from project root
            lua_echo_plugin_path = "scripts/plugins/echo.lua";
        }

        std::cout << "--- Test Paths ---" << std::endl;
        std::cout << "C++ Hello Plugin Path: " << (cpp_hello_plugin_path.empty() ? "NOT SET" : cpp_hello_plugin_path) << std::endl;
        std::cout << "C++ Echo Plugin Path: " << (cpp_echo_plugin_path.empty() ? "NOT SET" : cpp_echo_plugin_path) << std::endl;
        std::cout << "Lua Hello Plugin Path: " << lua_hello_plugin_path << std::endl;
        std::cout << "Lua Echo Plugin Path: " << lua_echo_plugin_path << std::endl;
        std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
        std::cout << "--- End Test Paths ---" << std::endl;
    }

    bool path_exists(const std::string& path) {
        return !path.empty() && std::filesystem::exists(path);
    }

    void execute_command(const std::string& command_name, const std::vector<std::string>& args = {}) {
        auto command = registry.get_command(command_name);
        if (command) {
            std::vector<std::string> full_args = {command_name};
            full_args.insert(full_args.end(), args.begin(), args.end());
            command->run(full_args);
        } else {
            FAIL() << "Command not found: " << command_name;
        }
    }
};

TEST_F(UnifiedPluginSystemTest, LoadAndUnloadLuaHelloPlugin) {
    ASSERT_TRUE(path_exists(lua_hello_plugin_path)) << "Lua hello plugin script not found at '" << lua_hello_plugin_path << "'. CWD: " << std::filesystem::current_path();

    EXPECT_TRUE(registry.load_plugin(lua_hello_plugin_path));
    ASSERT_NE(registry.get_command("hello_lua"), nullptr);

    {
        CoutRedirect redirect(captured_cout.rdbuf());
        execute_command("hello_lua");
    }
    EXPECT_EQ(captured_cout.str(), "Hello from Lua plugin!\n");
    captured_cout.str("");

    EXPECT_TRUE(registry.unload_plugin(lua_hello_plugin_path));
    EXPECT_EQ(registry.get_command("hello_lua"), nullptr);
}

TEST_F(UnifiedPluginSystemTest, LoadAndUnloadCppHelloPlugin) {
    if (!path_exists(cpp_hello_plugin_path)) {
        GTEST_SKIP() << "C++ Hello Plugin not found or path not set (TEST_CPP_HELLO_PLUGIN_PATH). Path: '" << cpp_hello_plugin_path << "'. CWD: " << std::filesystem::current_path() << ". Skipping test.";
        return;
    }

    EXPECT_TRUE(registry.load_plugin(cpp_hello_plugin_path));
    ASSERT_NE(registry.get_command("hello"), nullptr);

    {
        CoutRedirect redirect(captured_cout.rdbuf());
        execute_command("hello");
    }
    EXPECT_EQ(captured_cout.str(), "Hello from plugin!\n");
    captured_cout.str("");

    EXPECT_TRUE(registry.unload_plugin(cpp_hello_plugin_path));
    EXPECT_EQ(registry.get_command("hello"), nullptr);
}

TEST_F(UnifiedPluginSystemTest, LuaEchoPluginExecution) {
    ASSERT_TRUE(path_exists(lua_echo_plugin_path)) << "Lua echo plugin script not found at '" << lua_echo_plugin_path << "'. CWD: " << std::filesystem::current_path();

    EXPECT_TRUE(registry.load_plugin(lua_echo_plugin_path));
    ASSERT_NE(registry.get_command("echo_lua"), nullptr);

    {
        CoutRedirect redirect(captured_cout.rdbuf());
        execute_command("echo_lua", {"test", "message"});
    }
    EXPECT_EQ(captured_cout.str(), "test message\n");
    captured_cout.str("");

    EXPECT_TRUE(registry.unload_plugin(lua_echo_plugin_path));
    EXPECT_EQ(registry.get_command("echo_lua"), nullptr);
}

TEST_F(UnifiedPluginSystemTest, CppEchoPluginExecution) {
     if (!path_exists(cpp_echo_plugin_path)) {
        GTEST_SKIP() << "C++ Echo Plugin not found or path not set (TEST_CPP_ECHO_PLUGIN_PATH). Path: '" << cpp_echo_plugin_path << "'. CWD: " << std::filesystem::current_path() << ". Skipping test.";
        return;
    }

    EXPECT_TRUE(registry.load_plugin(cpp_echo_plugin_path));
    ASSERT_NE(registry.get_command("echo"), nullptr);

    {
        CoutRedirect redirect(captured_cout.rdbuf());
        execute_command("echo", {"cpp", "echo", "test"});
    }
    EXPECT_EQ(captured_cout.str(), "cpp echo test\n");
    captured_cout.str("");

    EXPECT_TRUE(registry.unload_plugin(cpp_echo_plugin_path));
    EXPECT_EQ(registry.get_command("echo"), nullptr);
}

TEST_F(UnifiedPluginSystemTest, LoadNonExistentPlugin) {
    std::string non_existent_lua = "path/to/nonexistent/plugin.lua";
    std::string non_existent_cpp = "path/to/nonexistent/plugin.so";
    EXPECT_FALSE(registry.load_plugin(non_existent_lua));
    EXPECT_FALSE(registry.load_plugin(non_existent_cpp));
}

TEST_F(UnifiedPluginSystemTest, UnloadNonLoadedPlugin) {
    EXPECT_FALSE(registry.unload_plugin("path/to/some/otherplugin.lua"));
}

TEST_F(UnifiedPluginSystemTest, MultiplePluginsSimultaneously) {
    ASSERT_TRUE(path_exists(lua_hello_plugin_path)) << "Lua hello plugin script not found at " << lua_hello_plugin_path;
    if (!path_exists(cpp_echo_plugin_path)) {
         GTEST_SKIP() << "C++ Echo Plugin not found or path not set. Path: " << cpp_echo_plugin_path << ". Skipping test for multiple plugins.";
        return;
    }

    EXPECT_TRUE(registry.load_plugin(lua_hello_plugin_path));
    EXPECT_TRUE(registry.load_plugin(cpp_echo_plugin_path));

    ASSERT_NE(registry.get_command("hello_lua"), nullptr);
    ASSERT_NE(registry.get_command("echo"), nullptr);

    {
        CoutRedirect redirect(captured_cout.rdbuf());
        execute_command("hello_lua");
        EXPECT_EQ(captured_cout.str(), "Hello from Lua plugin!\n");
        captured_cout.str("");

        execute_command("echo", {"test", "multiple"});
        EXPECT_EQ(captured_cout.str(), "test multiple\n");
        captured_cout.str("");
    }

    EXPECT_TRUE(registry.unload_plugin(lua_hello_plugin_path));
    EXPECT_EQ(registry.get_command("hello_lua"), nullptr);
    EXPECT_NE(registry.get_command("echo"), nullptr);

    EXPECT_TRUE(registry.unload_plugin(cpp_echo_plugin_path));
    EXPECT_EQ(registry.get_command("echo"), nullptr);
}

```
