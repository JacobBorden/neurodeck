#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include "command.hpp"
#include "tokenize.hpp"

// ------- registry builder (declared in command.cpp) -------------------------
std::unordered_map<std::string, std::unique_ptr<Command>> build_registry();



int main(){
    auto commands = build_registry();
    std::cout << "Welcome to Neurodeck shell! Type 'help' for a list of commands.\n";
    std::string input;

    bool running = true;
    while(running && std::cout<< "neurodeck> " && std::getline(std::cin, input)){
        auto tokens = tokenize(input);
        if(tokens.empty()) continue;

        
        auto it = commands.find(tokens[0]);
        if(it != commands.end()){
            if(tokens[0] == "exit") running = false;
            else it->second->run(tokens);
        } else {
            std::cout << "Unknown command: " << tokens[0] << "\n";
        }
    }
    std::cout << "Exiting Neurodeck shell. Goodbye!\n";
    return 0;
}