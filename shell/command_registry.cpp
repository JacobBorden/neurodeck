#include "command_registry.hpp"
#include "../core/plugin.hpp" // Required for Neurodeck::Plugin
#include "../core/lua_manager.hpp" // For LuaManager
#include "../core/lua_plugin.hpp"   // For LuaPlugin
#include <filesystem> // For path operations
#include <iostream>
#include <vector> // Required for destructor logic
#include <dlfcn.h>  // For dlopen, dlsym, dlclose, dlerror

// Assuming command implementations are in a 'commands' subdirectory
// and each command has its own header.
#include "commands/help.hpp"
#include "commands/exit.hpp"
#include "commands/open.hpp"
#include "commands/load_plugin_command.hpp"
#include "commands/unload_plugin_command.hpp"
#include "commands/exec_command.hpp"
#include "commands/lua_command.hpp"

namespace Neurodeck {

// Forward declare create_plugin_func and destroy_plugin_func types
using create_plugin_func = Neurodeck::Plugin* (*)();
using destroy_plugin_func = void (*)(Neurodeck::Plugin*);

// Wrapper for C++ plugins loaded via dlsym
class CppPluginWrapper : public Neurodeck::Plugin {
public:
    CppPluginWrapper(Neurodeck::Plugin* plugin_instance, destroy_plugin_func destroyer, void* lib_handle)
        : actual_plugin_(plugin_instance), destroy_fn_(destroyer), library_handle_(lib_handle) {
        if (!actual_plugin_) {
            throw std::runtime_error("CppPluginWrapper: actual_plugin_ cannot be null.");
        }
    }

    ~CppPluginWrapper() override {
        // Shutdown should have been called by CommandRegistry before this destructor
        if (actual_plugin_ && destroy_fn_) {
            // std::cout << "CppPluginWrapper: Destroying actual plugin via function pointer." << std::endl;
            destroy_fn_(actual_plugin_);
        } else if (actual_plugin_) {
            // Fallback if no destroy_fn_ was provided, though this indicates an issue with plugin loading
            // std::cerr << "CppPluginWrapper: Missing destroy_fn_ for actual plugin. Direct deleting (risky)." << std::endl;
            // delete actual_plugin_; // This is risky if not created with new.
        }
        actual_plugin_ = nullptr; // Avoid double deletion if destructor is somehow called twice

        if (library_handle_) {
            // std::cout << "CppPluginWrapper: Closing library handle." << std::endl;
            dlclose(library_handle_);
            library_handle_ = nullptr;
        }
    }

    std::string getName() const override {
        return actual_plugin_ ? actual_plugin_->getName() : "";
    }

    void initialize(Neurodeck::CommandRegistry* registry) override {
        if (actual_plugin_) actual_plugin_->initialize(registry);
    }

    void shutdown(Neurodeck::CommandRegistry* registry) override {
        if (actual_plugin_) actual_plugin_->shutdown(registry);
    }

private:
    Neurodeck::Plugin* actual_plugin_;
    destroy_plugin_func destroy_fn_;
    void* library_handle_;
};

} // namespace Neurodeck

// Note: Anonymous namespace for helper typedefs is removed as they are now in Neurodeck namespace or local to load_plugin

namespace Neurodeck {

CommandRegistry::CommandRegistry() {
    // Default constructor
}

CommandRegistry::~CommandRegistry() {
    std::cout << "CommandRegistry destructor: Shutting down and unloading plugins." << std::endl;
    std::vector<std::string> plugin_paths;
    // Collect paths to avoid iterator invalidation issues if shutdown modifies the map
    for (const auto& pair : loaded_plugins_) {
        plugin_paths.push_back(pair.first);
    }

    for (const std::string& path : plugin_paths) {
        auto it = loaded_plugins_.find(path);
        if (it != loaded_plugins_.end()) {
            Neurodeck::Plugin* raw_plugin = it->second.get(); // This is now likely CppPluginWrapper or LuaPlugin
            if (raw_plugin) {
                std::string pluginName = raw_plugin->getName().empty() ? path : raw_plugin->getName();
                std::cout << "Calling shutdown for plugin '" << pluginName << "' during CommandRegistry destruction." << std::endl;
                try {
                    raw_plugin->shutdown(this); // Calls shutdown on CppPluginWrapper or LuaPlugin
                } catch (const std::exception& e) {
                    std::cerr << "Error during shutdown of plugin " << pluginName << " in ~CommandRegistry: " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "Unknown error during shutdown of plugin " << pluginName << " in ~CommandRegistry." << std::endl;
                }
            }
        }
    }
    loaded_plugins_.clear(); // This will trigger destructors of CppPluginWrapper / LuaPlugin
    std::cout << "CommandRegistry destroyed." << std::endl;
}

void CommandRegistry::register_command(std::unique_ptr<Command> command) {
    if (!command) {
        std::cerr << "Error: Attempted to register a null command." << std::endl;
        return;
    }
    std::string command_name = command->name();
    if (commands_.count(command_name)) { // Use .count for clarity or .find as before
        std::cerr << "Error: Command '" << command_name << "' already registered. Ignoring new registration." << std::endl;
    } else {
        commands_[command_name] = std::move(command);
    }
}

void CommandRegistry::unregister_command(const std::string& command_name) {
    commands_.erase(command_name);
}

Command* CommandRegistry::get_command(const std::string& command_name) {
    auto it = commands_.find(command_name);
    if (it != commands_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<std::string> CommandRegistry::get_all_command_names() const {
    std::vector<std::string> names;
    names.reserve(commands_.size());
    for (const auto& pair : commands_) {
        names.push_back(pair.first);
    }
    return names;
}

bool CommandRegistry::load_plugin(const std::string& path) {
    std::filesystem::path plugin_fs_path(path);
    std::string extension = plugin_fs_path.extension().string();

    if (loaded_plugins_.count(path)) {
        std::cerr << "Plugin (C++ or Lua) from path '" << path << "' already loaded." << std::endl;
        return false;
    }

    if (extension == ".lua") {
        try {
            auto lua_manager = std::make_unique<Neurodeck::LuaManager>();
            if (!lua_manager->state()) {
                 std::cerr << "Failed to create Lua state for plugin: " << path << std::endl;
                 return false;
            }
            // LuaPlugin manages its LuaManager's lifetime.
            auto lua_plugin = std::make_unique<Neurodeck::LuaPlugin>(path, std::move(lua_manager));

            Neurodeck::Plugin* raw_plugin_ptr = lua_plugin.get();

            raw_plugin_ptr->initialize(this); // Call initialize

            // Store the LuaPlugin directly. std::unique_ptr<LuaPlugin> converts to std::unique_ptr<Plugin>.
            loaded_plugins_[path] = std::move(lua_plugin);

            std::cout << "Lua plugin '" << raw_plugin_ptr->getName() << "' from '" << path << "' loaded and initialized." << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load Lua plugin " << path << ": " << e.what() << std::endl;
            return false;
        }
    } else if (extension == ".so" || extension == ".dll" || extension == ".dylib") {
        dlerror(); // Clear old errors

        void* handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!handle) {
            std::cerr << "Cannot open C++ library " << path << ": " << dlerror() << std::endl;
            return false;
        }

        create_plugin_func create_fn = reinterpret_cast<create_plugin_func>(dlsym(handle, "create_plugin"));
        const char* dlsym_err = dlerror();
        if (dlsym_err) {
            std::cerr << "Cannot find symbol create_plugin in C++ plugin " << path << ": " << dlsym_err << std::endl;
            dlclose(handle);
            return false;
        }

        destroy_plugin_func destroy_fn = reinterpret_cast<destroy_plugin_func>(dlsym(handle, "destroy_plugin"));
        dlsym_err = dlerror(); // Capture error after dlsym for destroy_plugin
        if (dlsym_err) {
            std::cerr << "Cannot find symbol destroy_plugin in C++ plugin " << path << ": " << dlsym_err << std::endl;
            dlclose(handle);
            return false;
        }

        Neurodeck::Plugin* raw_plugin = create_fn();
        if (!raw_plugin) {
            std::cerr << "C++ plugin creation failed for " << path << std::endl;
            dlclose(handle);
            return false;
        }

        try {
            // Create the wrapper. The wrapper's destructor will handle cleanup.
            auto cpp_plugin_wrapper = std::make_unique<CppPluginWrapper>(raw_plugin, destroy_fn, handle);
            // Call initialize on the wrapped plugin through the wrapper
            cpp_plugin_wrapper->initialize(this);

            // Store the wrapper. std::unique_ptr<CppPluginWrapper> converts to std::unique_ptr<Plugin>.
            loaded_plugins_[path] = std::move(cpp_plugin_wrapper);

            std::cout << "C++ Plugin '" << raw_plugin->getName() << "' from '" << path << "' loaded and initialized." << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::string pluginName = raw_plugin ? raw_plugin->getName() : path; // Use the corrected version
            std::cerr << "Error initializing C++ plugin " << pluginName << ": " << e.what() << std::endl; // Use the corrected version
            // If wrapper construction or initialize failed, raw_plugin might exist.
            // We need to clean it up using destroy_fn and dlclose as the wrapper isn't in the map.
            if (raw_plugin && destroy_fn) {
                destroy_fn(raw_plugin);
            }
            dlclose(handle); // Always close handle if opened
            return false;
        } catch (...) {
            std::string pluginName = raw_plugin ? raw_plugin->getName() : path;
            std::cerr << "Unknown error initializing C++ plugin " << pluginName << "." << std::endl;
            if (raw_plugin && destroy_fn) {
                destroy_fn(raw_plugin);
            }
            dlclose(handle); // Always close handle if opened
            return false;
        }
    } else {
        std::cerr << "Unsupported plugin type: " << path << ". Must be .lua, .so, .dll, or .dylib." << std::endl;
        return false;
    }
}

bool CommandRegistry::unload_plugin(const std::string& path) {
    auto it = loaded_plugins_.find(path);
    if (it == loaded_plugins_.end()) {
        std::cerr << "Plugin from path '" << path << "' not found." << std::endl;
        return false;
    }

    Neurodeck::Plugin* plugin_ptr = it->second.get(); // Get raw pointer before moving/destroying
    std::string pluginName = plugin_ptr ? plugin_ptr->getName() : path;

    std::cout << "Unloading plugin '" << pluginName << "' from '" << path << "'." << std::endl;

    // Call shutdown before the plugin is removed from the map and its deleter is called.
    // The PluginDeleter will handle calling destroy_plugin and dlclose.
    try {
        if(plugin_ptr) {
            plugin_ptr->shutdown(this);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during shutdown of plugin " << pluginName << ": " << e.what() << std::endl;
        // Continue to unload even if shutdown fails
    } catch (...) {
        std::cerr << "Unknown error during shutdown of plugin " << pluginName << "." << std::endl;
        // Continue to unload
    }

    loaded_plugins_.erase(it); // This triggers the PluginDeleter

    std::cout << "Plugin '" << pluginName << "' resources released." << std::endl;
    return true;
}

// Implementation of the function to populate the registry with default commands.
void populate_default_commands(CommandRegistry& registry) {
    registry.register_command(std::make_unique<HelpCommand>(registry));
    registry.register_command(std::make_unique<ExitCommand>());
    registry.register_command(std::make_unique<OpenCommand>());
    registry.register_command(std::make_unique<LoadPluginCommand>(registry));
    registry.register_command(std::make_unique<UnloadPluginCommand>(registry));
    registry.register_command(std::make_unique<ExecCommand>());
    registry.register_command(std::make_unique<LuaCommand>());
    // If ChatCommand is a default command, register it here:
    // registry.register_command(std::make_unique<ChatCommand>());
}

} // namespace Neurodeck
