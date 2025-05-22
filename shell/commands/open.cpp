#include "shell/commands/open.hpp" // Added this include
#include "shell/command.hpp" // Base class is still needed
#include <iostream> // Not strictly needed for current open logic, but often included
#include <memory>
#include <vector>   // For std::vector in run method signature

// Class definition is now in the header, ensure implementation matches

std::string OpenCommand::name() const {
    return "open";
}

void OpenCommand::run(const std::vector<std::string>& args) {
    // Currently, the open command does nothing directly in its run method.
    // It's a stub.
    // (void)args; // Suppress unused parameter warning if desired
}

std::unique_ptr<Command> make_open() {
    return std::make_unique<OpenCommand>();
}