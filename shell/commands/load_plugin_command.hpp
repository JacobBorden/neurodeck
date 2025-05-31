#pragma once
#include "../command.hpp" // Adjusted path assuming commands/ is a subdirectory of shell/
#include "../command_registry.hpp" // Adjusted path
#include <string>
#include <vector>
#include <iostream> // For output

namespace Neurodeck {
class LoadPluginCommand : public Command {
private:
    CommandRegistry& registry_; // Reference to the command registry
public:
    LoadPluginCommand(CommandRegistry& registry) : registry_(registry) {}
    std::string name() const override { return "loadplugin"; }
    std::string description() const override { return "Loads a plugin from a shared library (.so, .dll) or Lua script (.lua) file. Usage: loadplugin <path_to_plugin_file>"; }
    void run(const std::vector<std::string>& args) override {
        if (args.size() < 2) {
            std::cerr << "Usage: " << name() << " <path_to_plugin_file>" << std::endl;
            return;
        }
        // Output from registry_.load_plugin is already handled by the method itself
        if (registry_.load_plugin(args[1])) {
            // std::cout << "Plugin '" << args[1] << "' loaded successfully." << std::endl; // Redundant with output in load_plugin
        } else {
            // std::cerr << "Failed to load plugin '" << args[1] << "'." << std::endl; // Redundant
        }
    }
};
} // namespace Neurodeck
