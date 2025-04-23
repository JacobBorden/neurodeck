#include "command.hpp"
#include <iostream>
#include <memory>

class ExitCommand : public Command {
    public:
    std::string name() const override {return "exit";}
    void run(const std::vector<std::string>& args) override {
       
        
    }
};

std::unique_ptr<Command> make_exit(){ return std::make_unique<ExitCommand>(); }