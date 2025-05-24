#pragma once
#include "../command.hpp" // Base class is still needed
#include <memory>
#include <string>
#include <vector>

namespace Neurodeck { // Added namespace

class LsCommand : public Neurodeck::Command { // Added Neurodeck::
public:
    std::string name() const override;
    std::string description() const override; // Added description
    void run(const std::vector<std::string>& args) override;
};

} // namespace Neurodeck

// Factory function - This is likely obsolete and should be removed
// as commands are now directly instantiated in populate_default_commands.
// For now, just ensure it compiles by adding the namespace if kept.
// std::unique_ptr<Neurodeck::Command> make_ls();
