#include "command.hpp"
#include <iostream>
#include <memory>

class ClearCommand : public Command {
    public:
    std::string name() const override {return "clear";}
    void run(const std::vector<std::string>& args) override {
        std::cout << "\033[2J\033[1;1H"; // ANSI escape code to clear the screen
        std::cout << "Screen cleared.\n";
    }
};

std::unique_ptr<Command> make_clear(){ return std::make_unique<ClearCommand>(); }