#include "command.hpp"
#include <memory>
#include <unordered_map>

// These extern declarations are for the actual command factory functions.
// When testing build_registry, we will provide mock implementations of these
// in the test file itself, which the linker should pick.
extern std::unique_ptr<Command> make_ls();
extern std::unique_ptr<Command> make_clear();
extern std::unique_ptr<Command> make_help();
extern std::unique_ptr<Command> make_exit();
extern std::unique_ptr<Command> make_open();

using CmdPtr = std::unique_ptr<Command>;
using Registry = std::unordered_map<std::string, CmdPtr>;

Registry build_registry() {
    Registry reg;
  auto add = [&](CmdPtr c){reg.emplace(c->name(), std::move(c));};
    add(make_ls());
    add(make_clear());
    add(make_help());
    add(make_exit());
    add(make_open());
    return reg;
}