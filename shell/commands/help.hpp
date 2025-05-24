#pragma once
#include "../command.hpp" // Base class is still needed
#include <memory>
#include <string>
#include <vector>

// Forward declaration for CommandRegistry
namespace Neurodeck { class CommandRegistry; }

namespace Neurodeck { // Added namespace

class HelpCommand : public Neurodeck::Command { // Added Neurodeck::
private:
    CommandRegistry& registry_; // Added registry reference
public:
    explicit HelpCommand(CommandRegistry& registry); // Added constructor
    std::string name() const override;
    std::string description() const override; // Added
    void run(const std::vector<std::string>& args) override;
};

} // namespace Neurodeck

// Factory function - Likely obsolete
// std::unique_ptr<Neurodeck::Command> make_help();
