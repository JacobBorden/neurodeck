#include "shell/commands/ls.hpp" // Added this include
#include "shell/command.hpp" // Base class is still needed
#include <iostream>
#include <memory>
#include <vector> // For std::vector in run method signature

// Class definition is now in the header, ensure implementation matches

std::string LsCommand::name() const {
    return "ls";
}

void LsCommand::run(const std::vector<std::string>& args) {
    // args is unused as per the command's behavior
    std::cout << "Available modules/apps:\n"
              << " notes  - textual note manager\n"
              << " calendar - calendar and event manager\n"
              << " ide - integrated development environment\n"
              << " calculator - simple calculator\n";
}

std::unique_ptr<Command> make_ls() {
    return std::make_unique<LsCommand>();
}