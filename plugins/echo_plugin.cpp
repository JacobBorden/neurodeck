#include <iostream>
#include <vector>
#include <string>
#include <vector>
#include <string>
#include <memory> // For std::make_unique

// Assuming 'plugins' is a subdirectory of the project root,
// and 'shell' is another subdirectory of the project root.
#include "../core/plugin.hpp" // Include the new Plugin interface
#include "../shell/command.hpp"
#include "../shell/command_registry.hpp"

namespace Neurodeck {

class EchoCommand : public Neurodeck::Command {
public:
    std::string name() const override { return "echo"; }
    std::string description() const override { return "Prints the arguments passed to it. From echo_plugin."; }
    void run(const std::vector<std::string>& args) override {
        // args[0] is the command name itself ("echo")
        if (args.size() > 1) {
            for (size_t i = 1; i < args.size(); ++i) {
                std::cout << args[i] << (i == args.size() - 1 ? "" : " ");
            }
        }
        std::cout << std::endl;
    }
};

} // namespace Neurodeck

// Plugin implementation
class EchoPlugin : public Neurodeck::Plugin {
public:
    std::string getName() const override {
        return "EchoPlugin";
    }

    void initialize(Neurodeck::CommandRegistry* registry) override {
        if (registry) {
            registry->register_command(std::make_unique<Neurodeck::EchoCommand>());
        }
    }

    void shutdown(Neurodeck::CommandRegistry* registry) override {
        if (registry) {
            registry->unregister_command("echo");
        }
    }
};

// Factory functions for the plugin
extern "C" {
    __attribute__((visibility("default")))
    Neurodeck::Plugin* create_plugin() {
        return new EchoPlugin();
    }

    __attribute__((visibility("default")))
    void destroy_plugin(Neurodeck::Plugin* plugin) {
        delete plugin;
    }
}
