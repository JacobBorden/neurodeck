/**
 * @file command_registry.hpp
 * @brief Defines the CommandRegistry class, central to managing shell commands.
 *
 * This file declares the Neurodeck::CommandRegistry class, which is responsible for
 * the registration, lifecycle management, and retrieval of shell commands.
 * Its design is fundamental for a modular shell architecture, supporting
 * not only statically defined commands but also paving the way for future
 * dynamic loading and unloading of commands as plugins or modules.
 */
#ifndef SHELL_COMMAND_REGISTRY_HPP
#define SHELL_COMMAND_REGISTRY_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream> // For error logging
#include <map>      // For std::map to store plugin handles

// Platform-specific includes for dynamic library loading
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

#include "command.hpp" // For Neurodeck::Command base class

namespace Neurodeck {

/**
 * @brief Manages the registration, lifecycle, and retrieval of shell commands.
 *
 * The CommandRegistry class serves as a central repository for all available commands
 * within the Neurodeck shell. It stores commands as unique pointers, thereby managing
 * their lifecycle. Commands are identified by a unique string name.
 * This class provides mechanisms to add (register) and remove (unregister) commands,
 * which is crucial for modularity and future support for dynamic command loading.
 * It also allows querying for available commands and retrieving them for execution.
 */
class CommandRegistry {
public:
    /**
     * @brief Default constructor.
     * Initializes an empty command registry, ready to have commands registered.
     */
    CommandRegistry();
    virtual ~CommandRegistry() = default;

    /**
     * @brief Registers a command with the registry.
     *
     * Adds the given command to the registry, taking ownership of it. The command's name,
     * obtained via `command->name()`, is used as the key for retrieval.
     * If a command with the same name already exists, an error message is logged to
     * `std::cerr`, and the new command is not added, preventing duplicates.
     * This method is critical for extending the shell's capabilities, both at startup
     * with built-in commands and potentially at runtime with dynamically loaded commands.
     *
     * @param command A `std::unique_ptr<Command>` to the command object to be registered.
     *                The registry takes ownership of the command.
     * @note Logs an error to `std::cerr` if `command` is null or if a command with the same name already exists.
     */
    void register_command(std::unique_ptr<Command> command);

    /**
     * @brief Unregisters a command from the registry.
     *
     * Removes the command associated with the given `command_name` from the registry.
     * This action also destroys the command object, as the registry owns it.
     * If no command with the specified name is found, the operation has no effect.
     * This is important for dynamic environments where commands might be unloaded.
     *
     * @param command_name The unique name of the command to unregister.
     */
    void unregister_command(const std::string& command_name);

    /**
     * @brief Retrieves a pointer to a command by its name.
     *
     * Looks up a command in the registry using its unique name.
     * The ownership of the command object remains with the CommandRegistry.
     * The returned pointer is valid as long as the command is registered.
     *
     * @param command_name The name of the command to retrieve.
     * @return Command* A raw pointer to the `Command` object if found; otherwise, `nullptr`.
     */
    Command* get_command(const std::string& command_name);

    /**
     * @brief Retrieves the names of all currently registered commands.
     *
     * This method is useful for features like a 'help' command that lists all
     * available commands, or for diagnostic purposes.
     *
     * @return std::vector<std::string> A vector containing the unique names of all
     *         currently registered commands. The order of names in the vector is not guaranteed.
     */
    std::vector<std::string> get_all_command_names() const;

    /**
     * @brief Loads a plugin from a shared library.
     *
     * Opens the specified shared library file (e.g., .so or .dll), looks for an
     * exported C function `register_commands(Neurodeck::CommandRegistry*)`, and calls it,
     * passing `this` (the current CommandRegistry instance) to allow the plugin
     * to register its commands.
     *
     * @param plugin_path The filesystem path to the plugin shared library.
     * @return true if the plugin was loaded and `register_commands` was successfully called,
     *         false otherwise (e.g., file not found, symbol not found, or other error).
     * @note Logs errors to `std::cerr` on failure.
     */
    virtual bool load_plugin(const std::string& plugin_path);

    /**
     * @brief Unloads a previously loaded plugin.
     *
     * Looks for an exported C function `unregister_commands(Neurodeck::CommandRegistry*)`
     * in the plugin and calls it. Then, it closes the shared library.
     * Commands previously registered by this plugin are expected to be unregistered
     * by the plugin's `unregister_commands` function. The registry itself does not
     * automatically track which commands belong to which plugin after they are registered.
     *
     * @param plugin_path The filesystem path of the plugin that was used to load it.
     * @return true if the plugin was successfully unloaded, false otherwise
     *         (e.g., plugin not found in loaded list, error during unload).
     * @note Logs errors to `std::cerr` on failure.
     */
    virtual bool unload_plugin(const std::string& plugin_path);

private:
    /// @brief The core storage for commands. Maps command names (string) to
    ///        `std::unique_ptr<Command>`, managing command lifecycle.
    std::unordered_map<std::string, std::unique_ptr<Command>> commands_;

    /// @brief Stores handles to loaded plugins. Maps plugin path to the OS-specific library handle.
    /// This is necessary to correctly unload plugins later.
    #if defined(_WIN32) || defined(_WIN64)
        std::map<std::string, HMODULE> loaded_plugin_handles_;
    #else
        std::map<std::string, void*> loaded_plugin_handles_;
    #endif
};

/**
 * @brief Populates the command registry with all statically defined, built-in commands.
 *
 * This function serves as the entry point for initializing the shell with its core
 * set of commands. It creates instances of each built-in command (e.g., `ls`,
 * `exit`, `help`) and registers them with the provided `CommandRegistry` instance.
 * This ensures that all essential commands are available from the start of the shell session.
 *
 * @param registry A reference to the `CommandRegistry` instance that will be populated
 *                 with the default set of commands.
 */
void populate_default_commands(CommandRegistry& registry);

} // namespace Neurodeck

#endif // SHELL_COMMAND_REGISTRY_HPP
