#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

//Helper function to split intput into tokens
std::vector<std::string> tokenize(const std::string &input) {
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

//Prints a simple help message listing available commands
void printHelp(){
    std::cout << "Available commands:\n"
                <<" ls - List files in the current directory\n"
                <<" clear - Clear the screen\n"
                <<" open <filename> - Open a file\n"
                <<" exit - Exit the shell\n"
                << " help - Show this help message\n";
}

//Simulate listing available modules/apps.
void commandLs() {
    std::cout << "Available modules/apps:\n"
              << " notes  - textual note manager\n"
              << " calendar - calendar and event manager\n"
              << " ide - integrated development environment\n"
              << " calculator - simple calculator\n";
}

//Simulate clearing the screen
void commandClear() {
    std::cout << "\033[2J\033[1;1H"; // ANSI escape code to clear the screen
}

//Placeholder for open [app] command
void commandOpen(const std::vector<std::string> &tokens) {
    if (tokens.size() < 2) {
        std::cout << "Usage: open [app]\n";
        return;
    }
    std::string app = tokens[1];
    //Convert to lowercase for consistency
    std::transform(app.begin(), app.end(), app.begin(), ::tolower);
    if (app == "notes") {
        std::cout << "Opening notes app...\n";
        //TODO: Implement notes app functionality
    } else if (app == "calendar") {
        std::cout << "Opening calendar app...\n";
        //TODO: Implement calendar app functionality
    } else if (app == "ide") {
        std::cout << "Opening IDE...\n";
        //TODO: Implement IDE functionality
    } else if (app == "calculator") {
        std::cout << "Opening calculator app...\n";
        //TODO: Implement calculator app functionality
    } else {
        std::cout << "Unknown app: " << app << "\n";
    }
}

//The main REPL shell loop
void runShell(){
    std::string input;
    std::cout<<"Welcome to Neurodeck Shell\n";
    printHelp();

    while(true){
        //Displat a prompt that feels like a shell.
        std::cout << "[neurodeck]> ";
        std::getline(std::cin, input);
        //Handle end of file (Ctrl+D) gracefully
        if (std::cin.eof()) {
            std::cout << "\nExiting shell.\n";
            break;
        }
        //Tokenize the input
        std::vector<std::string> tokens = tokenize(input);
        //Handle empty input
        if (tokens.empty()) {
            continue;
        }

        //Check whihch command was entered
        std::string command  = tokens[0];

        if(command == "help"){
            printHelp();
        } else if(command == "ls"){
            commandLs();
        } else if(command == "clear"){
            commandClear();
        } else if(command == "open"){
            commandOpen(tokens);
        } else if(command == "exit"){
            std::cout << "Exiting shell.\n";
            break;
        } else {
            std::cout << "Unknown command: " << command << "\n";
        }
    }
}

int main(){
    //Run the shell
    runShell();
    return 0;
}