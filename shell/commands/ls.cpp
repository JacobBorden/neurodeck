#include "ls.hpp"
// #include "../command.hpp" // Already included via ls.hpp if ls.hpp includes ../command.hpp
#include <iostream>
#include <memory> // For std::make_unique
#include <vector>

namespace Neurodeck { // Added namespace

std::string LsCommand::name() const {
    return "ls";
}

std::string LsCommand::description() const {
    // Provide a suitable description for the 'ls' command.
    // The previous output was more like a placeholder.
    // For now, let's describe it as listing available modules,
    // though a real 'ls' would list files.
    return "Lists available modules/apps (placeholder behavior).";
}

void LsCommand::run(const std::vector<std::string>& args) {
    // args is unused as per the command's behavior
    // The output can remain as placeholder or be changed to actual file listing if implemented.
    std::cout << "Available modules/apps (placeholder for ls command):\n"
              << " notes      - textual note manager\n"
              << " calendar   - calendar and event manager\n"
              << " ide        - integrated development environment\n"
              << " calculator - simple calculator\n";
}

} // namespace Neurodeck

// Factory function - This is likely obsolete.
/*
std::unique_ptr<Neurodeck::Command> make_ls() { // Added Neurodeck::
    return std::make_unique<Neurodeck::LsCommand>();
}
*/