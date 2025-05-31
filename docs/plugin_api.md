# Plugin API Documentation

This document describes how to create plugins for the Neurodeck application.
Plugins can be written in C++ or Lua.

## Core Concepts

All plugins, whether C++ or Lua, provide a set of commands that can be invoked from the application's shell. The plugin system is managed by the `CommandRegistry`, which handles loading, initializing, and unloading plugins.

### Plugin Lifecycle

1.  **Loading**: When `loadplugin <path_to_plugin_file>` is executed, the `CommandRegistry` determines if it's a C++ or Lua plugin based on the file extension.
2.  **Instantiation**:
    *   For C++ plugins, a factory function (`create_plugin`) is called.
    *   For Lua plugins, a `LuaPlugin` C++ object is created, which then executes the Lua script.
3.  **Initialization**: The `initialize(CommandRegistry* registry)` method (C++) or `initialize()` Lua function is called. Plugins should register their commands here.
4.  **Execution**: Registered commands can be invoked from the shell.
5.  **Shutdown**: When `unloadplugin <path_to_plugin_file>` is executed (or when the application shuts down), the `shutdown(CommandRegistry* registry)` method (C++) or `shutdown()` Lua function is called. Plugins should unregister their commands and perform cleanup.
6.  **Destruction**:
    *   For C++ plugins, a factory function (`destroy_plugin`) is called.
    *   For Lua plugins, the `LuaPlugin` C++ object is destroyed, cleaning up Lua resources.

## Creating C++ Plugins

C++ plugins are compiled into shared libraries (e.g., `.so` on Linux, `.dll` on Windows).

### 1. Implement the `Plugin` Interface

Your plugin must define a class that inherits from `Neurodeck::Plugin` (defined in `core/plugin.hpp`).

```cpp
// my_cpp_plugin.cpp
#include "core/plugin.hpp"
#include "shell/command.hpp"
#include "shell/command_registry.hpp"
#include <iostream>
#include <memory> // For std::make_unique

namespace MyNamespace {

// Define your command
class MyCppCommand : public Neurodeck::Command {
public:
    std::string name() const override { return "my_cpp_cmd"; }
    std::string description() const override { return "A command from MyCppPlugin."; }
    void run(const std::vector<std::string>& args) override {
        std::cout << "MyCppCommand running!" << std::endl;
        // Process args if needed
    }
};

// Define your plugin class
class MyCppPlugin : public Neurodeck::Plugin {
public:
    std::string getName() const override {
        return "MyCppPlugin";
    }

    void initialize(Neurodeck::CommandRegistry* registry) override {
        if (registry) {
            registry->register_command(std::make_unique<MyCppCommand>());
            std::cout << getName() << " initialized and command registered." << std::endl;
        }
    }

    void shutdown(Neurodeck::CommandRegistry* registry) override {
        if (registry) {
            registry->unregister_command("my_cpp_cmd");
            std::cout << getName() << " shutdown and command unregistered." << std::endl;
        }
    }
};

} // namespace MyNamespace

// Export C-style factory functions
extern "C" {
    __attribute__((visibility("default")))
    Neurodeck::Plugin* create_plugin() {
        return new MyNamespace::MyCppPlugin();
    }

    __attribute__((visibility("default")))
    void destroy_plugin(Neurodeck::Plugin* plugin) {
        delete plugin;
    }
}
```

### 2. Compile as a Shared Library

Use CMake or your preferred build system to compile this into a shared library (e.g., `libmy_cpp_plugin.so`).

```cmake
# Example CMakeLists.txt for a C++ plugin
cmake_minimum_required(VERSION 3.10)
project(MyCppPlugin)

set(CMAKE_CXX_STANDARD 17)

# Assuming Neurodeck source is in a directory accessible via relative path or find_package
# Add include directories for core and shell
include_directories(../../core ../../shell) # Adjust path as needed

add_library(my_cpp_plugin SHARED my_cpp_plugin.cpp)

# Link against Neurodeck libraries if necessary (usually not for simple plugins,
# as Command/CommandRegistry are interfaces)
```

### 3. Loading

Load with `loadplugin path/to/libmy_cpp_plugin.so`.

## Creating Lua Plugins

Lua plugins are single `.lua` script files.

### 1. Script Structure

A Lua plugin script should define `initialize()` and `shutdown()` functions. It can optionally define `get_plugin_name()`. To register and unregister commands, it uses global functions `neurodeck_register_command` and `neurodeck_unregister_command` provided by the C++ host.

```lua
-- my_lua_plugin.lua

local plugin_name = "MyLuaPlugin"

-- Optional: Define a function to explicitly provide the plugin name to C++
function get_plugin_name()
    return plugin_name
end

-- Define the function that your command will execute
local function my_lua_command_run(args_table)
    -- args_table is a numerically indexed table (array-like)
    -- args_table[1] is the command name itself (e.g., "my_lua_cmd")
    print(plugin_name .. ": my_lua_command_run called!")
    for i, v in ipairs(args_table) do
        print("Arg " .. i .. ": " .. v)
    end
end

-- Called when the plugin is loaded
function initialize()
    print(plugin_name .. ": Initializing...")
    -- Register a command: neurodeck_register_command(command_name, description, run_function)
    neurodeck_register_command("my_lua_cmd", "A command from MyLuaPlugin.", my_lua_command_run)
    print(plugin_name .. ": 'my_lua_cmd' command registered.")
end

-- Called when the plugin is unloaded
function shutdown()
    print(plugin_name .. ": Shutting down...")
    -- Unregister the command
    neurodeck_unregister_command("my_lua_cmd")
    print(plugin_name .. ": 'my_lua_cmd' command unregistered.")
end

print(plugin_name .. " script has been loaded by the Lua interpreter.")
```

### 2. API provided to Lua

*   `neurodeck_register_command(name_string, description_string, run_function)`:
    *   `name_string`: The name of the command to register.
    *   `description_string`: A short description of the command.
    *   `run_function`: A Lua function that will be called when the command is executed. This function receives one argument: a table containing the command name and its arguments as strings (1-indexed).
*   `neurodeck_unregister_command(name_string)`:
    *   `name_string`: The name of the command to unregister.

### 3. Loading

Load with `loadplugin path/to/my_lua_plugin.lua`.

## Command Naming

It's recommended to prefix commands with the plugin name or a unique identifier if there's a chance of collision with core commands or other plugins (e.g., `myplugin_do_stuff`). The examples `hello_lua` and `echo_lua` follow this for clarity.

```
