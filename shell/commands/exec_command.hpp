#pragma once

#include "../command.hpp" // Adjusted include path
#include <string>
#include <vector>

namespace Neurodeck {

class ExecCommand : public Command {
public:
    ExecCommand();
    std::string name() const override;
    std::string description() const override;
    void run(const std::vector<std::string>& args) override;
};

} // namespace Neurodeck
