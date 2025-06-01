#include "lua_command_wrapper.hpp"
#include <iostream>

namespace Neurodeck {

LuaCommandWrapper::LuaCommandWrapper(std::string name, std::string description, lua_State* L_plugin, std::string lua_function_key)
    : name_(std::move(name)),
      description_(std::move(description)),
      L_plugin_(L_plugin),
      lua_function_key_(std::move(lua_function_key)) {
    if (!L_plugin_) {
        throw std::runtime_error("LuaCommandWrapper: lua_State is null for command " + name_);
    }
}

LuaCommandWrapper::~LuaCommandWrapper() {
    // Clean up the Lua function from the Lua registry to prevent dangling references
    if (L_plugin_) {
        lua_pushnil(L_plugin_); // Push nil
        lua_setfield(L_plugin_, LUA_REGISTRYINDEX, lua_function_key_.c_str()); // registry[key] = nil
    }
}

std::string LuaCommandWrapper::name() const {
    return name_;
}

std::string LuaCommandWrapper::description() const {
    return description_;
}

void LuaCommandWrapper::run(const std::vector<std::string>& args) {
    if (!L_plugin_) {
        std::cerr << "LuaCommandWrapper: Cannot run '" << name_ << "', Lua state unavailable." << std::endl;
        return;
    }
    lua_getfield(L_plugin_, LUA_REGISTRYINDEX, lua_function_key_.c_str());
    if (!lua_isfunction(L_plugin_, -1)) {
        std::cerr << "LuaCommandWrapper: Lua function '" << lua_function_key_ << "' not found for command '" << name_ << "'." << std::endl;
        lua_pop(L_plugin_, 1); // Pop non-function
        return;
    }

    lua_newtable(L_plugin_); // Argument table
    for (size_t i = 0; i < args.size(); ++i) {
        lua_pushstring(L_plugin_, args[i].c_str());
        lua_rawseti(L_plugin_, -2, i + 1); // Lua tables are 1-indexed
    }

    if (lua_pcall(L_plugin_, 1, 0, 0) != LUA_OK) { // 1 arg (table), 0 results, no error handler
        std::cerr << "LuaCommandWrapper: Error running Lua for command '" << name_ << "': " << lua_tostring(L_plugin_, -1) << std::endl;
        lua_pop(L_plugin_, 1); // Pop error message
    }
}

} // namespace Neurodeck
