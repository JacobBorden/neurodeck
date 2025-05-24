#include "help.hpp"
#include "../command_registry.hpp" // Needed for CommandRegistry
#include <iostream>
#include <vector>
#include <algorithm> // For std::sort
#include <iomanip>   // For std::setw

// Constructor implementation (should be in help.cpp if not header-only)
// Assuming help.hpp has:
// namespace Neurodeck {
// class HelpCommand : public Command {
// private:
//     CommandRegistry& registry_;
// public:
//     HelpCommand(CommandRegistry& registry);
//     std::string name() const override;
//     std::string description() const override { return "Shows this help message, listing all available commands."; }
//     void run(const std::vector<std::string>& args) override;
// };
// }

namespace Neurodeck {

// If HelpCommand constructor is not defined in the header, it should be here:
HelpCommand::HelpCommand(CommandRegistry& registry) : registry_(registry) {}

std::string HelpCommand::name() const {
    return "help";
}

std::string HelpCommand::description() const {
    // This was added to the instructions as a good practice.
    return "Shows this help message, listing all available commands and their descriptions.";
}

void HelpCommand::run(const std::vector<std::string>& args) {
    (void)args; // args is unused for this command
    std::cout << "Available commands:\n";
    
    std::vector<std::string> command_names = registry_.get_all_command_names();
    std::sort(command_names.begin(), command_names.end()); // Sort alphabetically

    // Determine the maximum command name length for alignment
    size_t max_name_length = 0;
    for (const auto& cmd_name : command_names) {
        if (cmd_name.length() > max_name_length) {
            max_name_length = cmd_name.length();
        }
    }

    for (const auto& cmd_name : command_names) {
        Command* cmd = registry_.get_command(cmd_name);
        if (cmd) {
            // Print command name, padded for alignment, then its description
            std::cout << "  " << std::left << std::setw(max_name_length + 2) << cmd_name 
                      << "- " << cmd->description() << std::endl;
        }
    }
    std::cout << "\nType '<command_name> --help' for more information on a specific command if available (not implemented yet)." << std::endl;
}

} // namespace Neurodeck

// The make_help factory function is likely obsolete if commands are created directly
// in populate_default_commands, e.g. std::make_unique<HelpCommand>(registry)
// If it's still used, it would need the registry:
// std::unique_ptr<Command> make_help(CommandRegistry& registry) {
//     return std::make_unique<HelpCommand>(registry);
// }
// However, populate_default_commands now directly constructs HelpCommand with the registry.
// So, this factory function is likely no longer needed and might have been removed
// or should be removed if it's not used. For now, I will comment it out.
/*
std::unique_ptr<Command> make_help() {
    // This version of make_help is problematic as HelpCommand now needs CommandRegistry&
    // If this factory is still required, it must be updated or removed.
    // For now, assume it's not used as populate_default_commands directly constructs.
    // return std::make_unique<HelpCommand>(); // This would fail to compile if constructor requires registry
    return nullptr; // Placeholder to signify it needs update or removal
}
*/