#include <iostream>
#include <memory> // For std::make_unique

// Assuming 'plugins' is a subdirectory of the project root,
// and 'shell' is another subdirectory of the project root.
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

// These C-style functions are the interface for the dynamic library loader.
// They allow the main application to register and unregister commands from this plugin.
extern "C" {

    // Function to register commands from this plugin
    __attribute__((visibility("default"))) // Ensure symbol is exported on Linux/macOS
    void register_commands(Neurodeck::CommandRegistry* registry) {
        if (registry) {
            registry->register_command(std::make_unique<Neurodeck::HelloCommand>());
            // std::cout << "hello_plugin: HelloCommand registered." << std::endl; // Optional debug log
        } else {
            // std::cerr << "hello_plugin: Failed to register commands, null registry pointer." << std::endl; // Optional debug log
        }
    }

    // Function to unregister commands from this plugin
    __attribute__((visibility("default"))) // Ensure symbol is exported on Linux/macOS
    void unregister_commands(Neurodeck::CommandRegistry* registry) {
        if (registry) {
            registry->unregister_command("hello");
            // std::cout << "hello_plugin: HelloCommand unregistered." << std::endl; // Optional debug log
        } else {
            // std::cerr << "hello_plugin: Failed to unregister commands, null registry pointer." << std::endl; // Optional debug log
        }
    }

}
