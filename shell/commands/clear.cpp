#include "clear.hpp"
#include <iostream>
#include <memory> // For std::make_unique
#include <vector>

namespace Neurodeck { // Added namespace

std::string ClearCommand::name() const { return "clear"; }

std::string ClearCommand::description() const {
    return "Clears the terminal screen.";
}

void ClearCommand::run(const std::vector<std::string>& args) {
    // args is unused as per the command's behavior
    (void)args; // Explicitly mark args as unused
    std::cout << "\033[2J\033[1;1H"; // ANSI escape code to clear the screen
    // The "Screen cleared" message might be immediately wiped by the clear.
    // Depending on terminal behavior, it might be better to not print anything after clearing.
    // For now, keeping it for consistency if some terminals show it briefly.
    // std::cout << "Screen cleared.\n"; 
}

} // namespace Neurodeck

// Factory function - Likely obsolete
/*
std::unique_ptr<Neurodeck::Command> make_clear() { // Added Neurodeck::
    return std::make_unique<Neurodeck::ClearCommand>();
}
*/