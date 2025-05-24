// This file is intentionally left mostly blank.
// The old build_registry() function and associated factory function
// declarations that were previously here are now obsolete due to the
// introduction of the CommandRegistry class and populate_default_commands.
// Command definitions are in their respective command files or are header-only.
// The Command base class (in command.hpp) is purely abstract with
// a default virtual destructor, so it does not require definitions here.

// #include "command.hpp" // Not strictly needed if file is empty of code using Command.
// #include <memory>       // Not strictly needed.
// #include <unordered_map> // Not strictly needed.