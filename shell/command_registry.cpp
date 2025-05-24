#include "command_registry.hpp"
#include <iostream> // For std::cerr
#include <algorithm> // For std::remove (unused here, but was present)

// Platform-specific includes for dynamic library loading are already in .hpp
// but dlfcn.h might be needed here for RTLD_LAZY etc. if not transitively included.
#if !(defined(_WIN32) || defined(_WIN64))
    #include <dlfcn.h> // For RTLD_LAZY, dlerror
#endif

// Assuming command implementations are in a 'commands' subdirectory
// and each command has its own header.
#include "commands/ls.hpp"      // Corrected: was ls_command.hpp
#include "commands/clear.hpp"   // Corrected: was clear_command.hpp
#include "commands/help.hpp"    // Corrected: was help_command.hpp
#include "commands/exit.hpp"    // Corrected: was exit_command.hpp
#include "commands/open.hpp"    // Corrected: was open_command.hpp
#include "commands/load_plugin_command.hpp"
#include "commands/unload_plugin_command.hpp"
// Note: If the ChatCommand is also a default command, its header should be included here.
// #include "commands/chat_command.hpp"


namespace Neurodeck {

// Define function pointer types for plugin functions
typedef void (*register_commands_func_t)(Neurodeck::CommandRegistry*);
typedef void (*unregister_commands_func_t)(Neurodeck::CommandRegistry*);

CommandRegistry::CommandRegistry() = default;

void CommandRegistry::register_command(std::unique_ptr<Command> command) {
    if (!command) {
        std::cerr << "Error: Attempted to register a null command." << std::endl;
        return;
    }
    std::string command_name = command->name();
    if (commands_.find(command_name) != commands_.end()) {
        std::cerr << "Error: Command '" << command_name << "' already registered. Ignoring new registration." << std::endl;
    } else {
        commands_[command_name] = std::move(command);
        // std::cout << "Command '" << command_name << "' registered." << std::endl; // Optional: for debugging
    }
}

void CommandRegistry::unregister_command(const std::string& command_name) {
    auto it = commands_.find(command_name);
    if (it != commands_.end()) {
        commands_.erase(it);
        // std::cout << "Command '" << command_name << "' unregistered." << std::endl; // Optional: for debugging
    } else {
        // std::cerr << "Warning: Attempted to unregister non-existent command '" << command_name << "'." << std::endl; // Optional: for debugging
    }
}

Command* CommandRegistry::get_command(const std::string& command_name) {
    auto it = commands_.find(command_name);
    if (it != commands_.end()) {
        return it->second.get(); // Return raw pointer
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

// Implementation of the function to populate the registry with default commands.
void populate_default_commands(CommandRegistry& registry) {
    registry.register_command(std::make_unique<LsCommand>());
    registry.register_command(std::make_unique<ClearCommand>());
    registry.register_command(std::make_unique<HelpCommand>(registry)); 
    registry.register_command(std::make_unique<ExitCommand>());
    registry.register_command(std::make_unique<OpenCommand>());
    registry.register_command(std::make_unique<LoadPluginCommand>(registry));   // Added
    registry.register_command(std::make_unique<UnloadPluginCommand>(registry)); // Added
    // If ChatCommand is a default command, register it here:
    // registry.register_command(std::make_unique<ChatCommand>());
}


bool CommandRegistry::load_plugin(const std::string& plugin_path) {
    if (loaded_plugin_handles_.count(plugin_path)) {
        std::cerr << "Plugin already loaded: " << plugin_path << std::endl;
        return false;
    }

#if defined(_WIN32) || defined(_WIN64)
    HMODULE handle = LoadLibrary(plugin_path.c_str());
    if (!handle) {
        std::cerr << "Error loading plugin " << plugin_path << ": " << GetLastError() << std::endl;
        return false;
    }
#else // POSIX
    void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Error loading plugin " << plugin_path << ": " << dlerror() << std::endl;
        return false;
    }
#endif

    register_commands_func_t register_func = nullptr;
#if defined(_WIN32) || defined(_WIN64)
    register_func = (register_commands_func_t)GetProcAddress(handle, "register_commands");
#else
    register_func = (register_commands_func_t)dlsym(handle, "register_commands");
#endif

    if (!register_func) {
        std::cerr << "Error finding 'register_commands' in plugin " << plugin_path << ": ";
#if defined(_WIN32) || defined(_WIN64)
        std::cerr << GetLastError();
#else
        std::cerr << dlerror();
#endif
        std::cerr << std::endl;
#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    try {
        register_func(this);
        loaded_plugin_handles_[plugin_path] = handle;
        std::cout << "Plugin " << plugin_path << " loaded and commands registered." << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception while calling register_commands for plugin " << plugin_path << ": " << e.what() << std::endl;
#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    } catch (...) {
        std::cerr << "Unknown exception while calling register_commands for plugin " << plugin_path << std::endl;
#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }
}

bool CommandRegistry::unload_plugin(const std::string& plugin_path) {
    auto it = loaded_plugin_handles_.find(plugin_path);
    if (it == loaded_plugin_handles_.end()) {
        std::cerr << "Plugin not loaded or already unloaded: " << plugin_path << std::endl;
        return false;
    }

#if defined(_WIN32) || defined(_WIN64)
    HMODULE handle = it->second;
#else
    void* handle = it->second;
#endif

    unregister_commands_func_t unregister_func = nullptr;
#if defined(_WIN32) || defined(_WIN64)
    unregister_func = (unregister_commands_func_t)GetProcAddress(handle, "unregister_commands");
#else
    unregister_func = (unregister_commands_func_t)dlsym(handle, "unregister_commands");
#endif

    if (unregister_func) {
        try {
            unregister_func(this);
            std::cout << "Called 'unregister_commands' for plugin " << plugin_path << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Exception while calling unregister_commands for plugin " << plugin_path << ": " << e.what() << std::endl;
            // Decide if we should still attempt to unload the library
        } catch (...) {
            std::cerr << "Unknown exception while calling unregister_commands for plugin " << plugin_path << std::endl;
            // Decide if we should still attempt to unload the library
        }
    } else {
        std::cerr << "Warning: 'unregister_commands' not found in plugin " << plugin_path << ". Commands may not be cleaned up completely." << std::endl;
    }

#if defined(_WIN32) || defined(_WIN64)
    if (!FreeLibrary(handle)) {
        std::cerr << "Error unloading plugin " << plugin_path << ": " << GetLastError() << std::endl;
        return false;
    }
#else
    if (dlclose(handle) != 0) {
        std::cerr << "Error unloading plugin " << plugin_path << ": " << dlerror() << std::endl;
        return false;
    }
#endif

    loaded_plugin_handles_.erase(it);
    std::cout << "Plugin " << plugin_path << " unloaded." << std::endl;
    return true;
}

} // namespace Neurodeck
