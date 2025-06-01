#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include <string>
#include <vector>

// Forward declaration
namespace Neurodeck {
class CommandRegistry;
class Command; // Assuming Command is defined elsewhere, e.g., "../shell/command.hpp"
}

namespace Neurodeck {

class Plugin {
public:
    virtual ~Plugin() = default;

    // Returns the name of the plugin
    virtual std::string getName() const = 0;

    // Called when the plugin is loaded.
    // The plugin should register its commands with the provided registry.
    virtual void initialize(CommandRegistry* registry) = 0;

    // Called when the plugin is about to be unloaded.
    // The plugin should unregister its commands and perform any cleanup.
    virtual void shutdown(CommandRegistry* registry) = 0;
};

} // namespace Neurodeck

#endif // PLUGIN_HPP
