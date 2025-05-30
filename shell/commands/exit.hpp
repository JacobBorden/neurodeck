#pragma once
#include "../command.hpp" // Base class is still needed
#include <memory>
#include <string>
#include <vector>

class ExitCommand : public Command {
public:
    std::string name() const override;
    void run(const std::vector<std::string>& args) override;
};

// Factory function
std::unique_ptr<Command> make_exit();
