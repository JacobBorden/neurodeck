#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory> // For std::unique_ptr, if not already via other headers
#include "command.hpp"
#include "tokenize.hpp"
#include "command_registry.hpp" // Include the new CommandRegistry header

// The build_registry() function declaration is removed as command management
// is now handled by the CommandRegistry class.

/**
 * @brief Main entry point for the Neurodeck shell application.
 * 
 * Initializes the shell environment, sets up command handling through the
 * CommandRegistry, and enters the main input loop to process user commands.
 * The shell continues to run until an 'exit' command is issued or EOF is encountered.
 * @return int Exit status of the application; 0 for normal termination.
 */
int main(){
    // Instantiate the CommandRegistry, which will hold all available commands,
    // managing their lifecycle and providing access for execution.
    Neurodeck::CommandRegistry registry; 

    // Load all standard, built-in commands into the registry.
    // This makes commands like 'ls', 'help', 'exit', etc., available at startup.
    Neurodeck::populate_default_commands(registry); 

    std::cout << "Welcome to Neurodeck shell! Type 'help' for a list of commands.\n";
    std::string input;

    bool running = true;
    // The condition `std::cout << "neurodeck> "` is to ensure the prompt is displayed before each input.
    // std::getline returns true if input is successfully read, false on EOF or error.
    while(running && std::cout << "neurodeck> " && std::getline(std::cin, input)){
        auto tokens = tokenize(input);
        if(tokens.empty()) {
            // If the input is empty, just show the prompt again.
            continue;
        }

        // Retrieve the command from the registry based on the first token (command name).
        Neurodeck::Command* command = registry.get_command(tokens[0]); 
        
        if(command != nullptr){ // Check if the command exists in the registry
            // Special handling for 'exit' command to control the main loop's 'running' state.
            // This ensures the loop terminates correctly when 'exit' is entered.
            if(command->name() == "exit") { 
                running = false;
                // The ExitCommand's run() method might also perform cleanup or print messages.
            }
            // Execute the command.
            command->run(tokens); 
        } else {
            // If the command is not found, inform the user and suggest 'help'.
            std::cout << "Unknown command: " << tokens[0] << ". Type 'help' for a list of commands.\n";
        }
    }

    // Check if the loop exited due to EOF or an error other than explicit 'exit' command
    if (std::cin.eof()) {
        // If EOF is reached (e.g., Ctrl+D), print a newline for cleaner terminal output
        std::cout << std::endl; 
    }
    
    std::cout << "Exiting Neurodeck shell. Goodbye!\n";
    return 0;
}