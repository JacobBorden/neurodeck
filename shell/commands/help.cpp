#include "command.hpp"
#include <iostream>
#include <memory>

class HelpCommand : public Command {
    public:
    std::string name() const override {return "help";}
    void run(const std::vector<std::string>& args) override {
        std::cout << "Available commands:\n"
        <<" ls - List files in the current directory\n"
        <<" clear - Clear the screen\n"
        <<" open <filename> - Open a file\n"
        <<" exit - Exit the shell\n"
        << " help - Show this help message\n";
        
    }
};

std::unique_ptr<Command> make_help(){ return std::make_unique<HelpCommand>(); }