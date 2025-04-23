#include "command.hpp"
#include <iostream>
#include <memory>

class OpenCommand : public Command {
    public:
    std::string name() const override {return "open";}
    void run(const std::vector<std::string>& args) override {

    }
};

std::unique_ptr<Command> make_open(){ return std::make_unique<OpenCommand>(); }