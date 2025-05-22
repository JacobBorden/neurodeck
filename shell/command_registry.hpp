#ifndef SHELL_COMMAND_REGISTRY_HPP
#define SHELL_COMMAND_REGISTRY_HPP

#include <string>
#include <vector> // Included for completeness, though not directly used by build_registry signature
#include <memory> // For std::unique_ptr
#include <unordered_map> // For std::unordered_map
#include "shell/command.hpp" // For the Command base class

// Forward declaration if Command is not fully defined via command.hpp for some reason
// class Command; 

// Function to build and return the command registry.
// The registry maps command names (strings) to unique pointers of Command objects.
std::unordered_map<std::string, std::unique_ptr<Command>> build_registry();

#endif // SHELL_COMMAND_REGISTRY_HPP
