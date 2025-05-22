#include "exit.hpp" // Added this include
#include "../command.hpp" // Base class is still needed
#include <iostream> // Not strictly needed for current exit logic, but often included
#include <memory>
#include <vector>   // For std::vector in run method signature

// Class definition is now in the header, ensure implementation matches

std::string ExitCommand::name() const {
    return "exit";
}

void ExitCommand::run(const std::vector<std::string>& args) {
    // Currently, the exit command does nothing directly in its run method.
    // The actual exit logic is handled by the main shell loop when it sees this command.
    // So, this method remains empty.
    // (void)args; // Suppress unused parameter warning if desired
}

std::unique_ptr<Command> make_exit() {
    return std::make_unique<ExitCommand>();
}