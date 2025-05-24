#pragma once
#include "../command.hpp" // Adjusted path
#include "../command_registry.hpp" // Adjusted path
#include <string>
#include <vector>
#include <iostream>

namespace Neurodeck {
class UnloadPluginCommand : public Command {
private:
    CommandRegistry& registry_;
public:
    UnloadPluginCommand(CommandRegistry& registry) : registry_(registry) {}
    std::string name() const override { return "unloadplugin"; }
    std::string description() const override { return "Unloads a plugin. Usage: unloadplugin <path_to_plugin_file>"; }
    void run(const std::vector<std::string>& args) override {
        if (args.size() < 2) {
            std::cerr << "Usage: " << name() << " <path_to_plugin_file>" << std::endl;
            return;
        }
        // Output from registry_.unload_plugin is already handled by the method itself
        if (registry_.unload_plugin(args[1])) {
            // std::cout << "Plugin '" << args[1] << "' unloaded successfully." << std::endl; // Redundant
        } else {
            // std::cerr << "Failed to unload plugin '" << args[1] << "'." << std::endl; // Redundant
        }
    }
};
} // namespace Neurodeck
