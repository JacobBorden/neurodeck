#include <iostream>
#include <iostream>
#include <memory> // For std::make_unique

// Assuming 'plugins' is a subdirectory of the project root,
// and 'shell' is another subdirectory of the project root.
#include "../core/plugin.hpp" // Include the new Plugin interface
#include "../shell/command.hpp"
#include "../shell/command_registry.hpp"

namespace Neurodeck {

class HelloCommand : public Neurodeck::Command {
public:
    std::string name() const override { return "hello"; }
    std::string description() const override { return "Prints a greeting message from the hello_plugin."; }
    void run(const std::vector<std::string>& args) override {
        // args[0] is the command name itself ("hello")
        // This command doesn't take any additional arguments.
        if (args.size() > 1) {
            std::cout << "Hello plugin: The 'hello' command does not take any arguments." << std::endl;
        } else {
            std::cout << "Hello from plugin!" << std::endl;
        }
    }
};

} // namespace Neurodeck

// Plugin implementation
class HelloPlugin : public Neurodeck::Plugin {
public:
    std::string getName() const override {
        return "HelloPlugin";
    }

    void initialize(Neurodeck::CommandRegistry* registry) override {
        if (registry) {
            registry->register_command(std::make_unique<Neurodeck::HelloCommand>());
        }
    }

    void shutdown(Neurodeck::CommandRegistry* registry) override {
        if (registry) {
            registry->unregister_command("hello");
        }
    }
};

// Factory functions for the plugin
extern "C" {
    __attribute__((visibility("default")))
    Neurodeck::Plugin* create_plugin() {
        return new HelloPlugin();
    }

    __attribute__((visibility("default")))
    void destroy_plugin(Neurodeck::Plugin* plugin) {
        delete plugin;
    }
}
