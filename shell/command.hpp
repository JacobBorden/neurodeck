#pragma once
#include <string>
#include <vector>

namespace Neurodeck {

class Command {
public:
    virtual ~Command() = default;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0; // Added based on HelpCommand expectation
    virtual void run(const std::vector<std::string>& args) = 0;
};

} // namespace Neurodeck
