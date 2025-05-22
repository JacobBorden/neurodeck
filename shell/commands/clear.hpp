#pragma once
#include "shell/command.hpp" 
#include <memory>
#include <string>
#include <vector>

class ClearCommand : public Command {
public:
    std::string name() const override;
    void run(const std::vector<std::string>& args) override;
};

// Factory function
std::unique_ptr<Command> make_clear();
