#pragma once
#include "../command.hpp" // Base class is still needed
#include <memory>
#include <string>
#include <vector>

namespace Neurodeck { // Added namespace

class OpenCommand : public Neurodeck::Command { // Added Neurodeck::
public:
    std::string name() const override;
    std::string description() const override; // Added
    void run(const std::vector<std::string>& args) override;
};

} // namespace Neurodeck

// Factory function - Likely obsolete
// std::unique_ptr<Neurodeck::Command> make_open();
