#include "clear.hpp" // Added this include
#include "../command.hpp" // Base class is still needed
#include <iostream>
#include <memory>
#include <vector> // For std::vector in run method signature

// Class definition is now in the header, ensure implementation matches

std::string ClearCommand::name() const {return "clear";}

void ClearCommand::run(const std::vector<std::string>& args) {
    // args is unused as per the command's behavior
    std::cout << "\033[2J\033[1;1H"; // ANSI escape code to clear the screen
    std::cout << "Screen cleared.\n";
}

std::unique_ptr<Command> make_clear(){ return std::make_unique<ClearCommand>(); }