#include "command.hpp"
#include <iostream>
#include <memory>

class LsCommand : public Command {
    public:
    std::string name() const override {return "ls";}
    void run(const std::vector<std::string>& args) override {
        std::cout << "Available modules/apps:\n"
        << " notes  - textual note manager\n"
        << " calendar - calendar and event manager\n"
        << " ide - integrated development environment\n"
        << " calculator - simple calculator\n";
    }
};

std::unique_ptr<Command> make_ls(){ return std::make_unique<LsCommand>(); }