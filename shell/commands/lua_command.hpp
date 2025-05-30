#pragma once

#include "../command.hpp"
#include "../../core/lua_manager.hpp" // Include LuaManager header
#include <string>
#include <vector>
#include <memory> // For std::unique_ptr

namespace Neurodeck {

class LuaCommand : public Command {
public:
    LuaCommand();
    std::string name() const override;
    std::string description() const override;
    void run(const std::vector<std::string>& args) override;

private:
    std::unique_ptr<LuaManager> luaManager_; 
};

} // namespace Neurodeck
