#include <iostream>
#include <vector>
#include <string>
#include <memory> // For std::make_unique

// Assuming 'plugins' is a subdirectory of the project root,
// and 'shell' is another subdirectory of the project root.
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

// These C-style functions are the interface for the dynamic library loader.
// They allow the main application to register and unregister commands from this plugin.
extern "C" {

    // Function to register commands from this plugin
    __attribute__((visibility("default"))) // Ensure symbol is exported on Linux/macOS
    void register_commands(Neurodeck::CommandRegistry* registry) {
        if (registry) {
            registry->register_command(std::make_unique<Neurodeck::EchoCommand>());
            // std::cout << "echo_plugin: EchoCommand registered." << std::endl; // Optional debug log
        } else {
            // std::cerr << "echo_plugin: Failed to register commands, null registry pointer." << std::endl; // Optional debug log
        }
    }

    // Function to unregister commands from this plugin
    __attribute__((visibility("default"))) // Ensure symbol is exported on Linux/macOS
    void unregister_commands(Neurodeck::CommandRegistry* registry) {
        if (registry) {
            registry->unregister_command("echo");
            // std::cout << "echo_plugin: EchoCommand unregistered." << std::endl; // Optional debug log
        } else {
            // std::cerr << "echo_plugin: Failed to unregister commands, null registry pointer." << std::endl; // Optional debug log
        }
    }

}
