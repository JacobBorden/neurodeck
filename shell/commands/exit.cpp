#include "exit.hpp"
#include <iostream>
#include <memory> // For std::make_unique
#include <vector>

namespace Neurodeck { // Added namespace

std::string ExitCommand::name() const {
    return "exit";
}

std::string ExitCommand::description() const {
    return "Exits the Neurodeck shell.";
}

void ExitCommand::run(const std::vector<std::string>& args) {
    // The main shell loop handles the 'running = false' logic when 'exit' is detected by name.
    // This run method can be used for any cleanup or messages before exiting.
    (void)args; // Suppress unused parameter warning
    std::cout << "Exiting Neurodeck shell..." << std::endl; // Optional: message from command itself
}

} // namespace Neurodeck

// Factory function - Likely obsolete
/*
std::unique_ptr<Neurodeck::Command> make_exit() { // Added Neurodeck::
    return std::make_unique<Neurodeck::ExitCommand>();
}
*/