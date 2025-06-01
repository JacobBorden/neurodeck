#ifndef LUA_COMMAND_WRAPPER_HPP
#define LUA_COMMAND_WRAPPER_HPP

#include "../shell/command.hpp"
#include "lua_manager.hpp" // For lua_State
#include <string>
#include <vector>

#ifdef LUA_FOUND_IN_CMAKE
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#endif

namespace Neurodeck {

class LuaCommandWrapper : public Command {
public:
    LuaCommandWrapper(std::string name, std::string description, lua_State* L_plugin, std::string lua_function_key);
    ~LuaCommandWrapper() override;

    std::string name() const override;
    std::string description() const override;
    void run(const std::vector<std::string>& args) override;

private:
    std::string name_;
    std::string description_;
    lua_State* L_plugin_; // Non-owning pointer to the plugin's Lua state
    std::string lua_function_key_; // Key to retrieve Lua function from L_plugin_'s registry
};

} // namespace Neurodeck

#endif // LUA_COMMAND_WRAPPER_HPP
