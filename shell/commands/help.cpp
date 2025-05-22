#include "help.hpp" // Added this include
#include "../command.hpp" // Base class is still needed
#include <iostream>
#include <memory>
#include <vector> // For std::vector in run method signature

// Class definition is now in the header, ensure implementation matches

std::string HelpCommand::name() const {
    return "help";
}

void HelpCommand::run(const std::vector<std::string>& args) {
    // args is unused as per the command's behavior
    std::cout << "Available commands:\n"
              << " ls - List files in the current directory\n"
              << " clear - Clear the screen\n"
              << " open <filename> - Open a file\n"
              << " exit - Exit the shell\n"
              << " help - Show this help message\n";
}

std::unique_ptr<Command> make_help() {
    return std::make_unique<HelpCommand>();
}