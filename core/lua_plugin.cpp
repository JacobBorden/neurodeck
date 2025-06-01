#include "lua_plugin.hpp"
#include "../shell/command_registry.hpp" // Adjusted path
#include "lua_command_wrapper.hpp"
#include <iostream>
#include <filesystem>

namespace Neurodeck {

// Forward declaration of C functions to be exposed to Lua
static int lua_neurodeck_register_command(lua_State* L);
static int lua_neurodeck_unregister_command(lua_State* L);

LuaPlugin::LuaPlugin(std::string script_path, std::unique_ptr<LuaManager> lua_manager)
    : script_path_(std::move(script_path)),
      lua_manager_(std::move(lua_manager)),
      command_registry_ptr_(nullptr) {
    if (!lua_manager_ || !lua_manager_->state()) {
        throw std::runtime_error("LuaPlugin: LuaManager not initialized or Lua state is null.");
    }
    plugin_name_ = std::filesystem::path(script_path_).stem().string();
    lua_State* L = lua_manager_->state();

    // Push `this` (LuaPlugin*) as a light userdata to be used as an upvalue for the C functions
    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, lua_neurodeck_register_command, 1);
    lua_setglobal(L, "neurodeck_register_command");

    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, lua_neurodeck_unregister_command, 1);
    lua_setglobal(L, "neurodeck_unregister_command");

    if (luaL_dofile(L, script_path_.c_str()) != LUA_OK) {
        const char* error_msg = lua_tostring(L, -1);
        std::string err = "LuaPlugin: Failed to load script " + script_path_ + ": " + (error_msg ? error_msg : "Unknown error");
        lua_pop(L, 1);
        throw std::runtime_error(err);
    }
}

LuaPlugin::~LuaPlugin() {
    // If shutdown wasn't called or didn't clean up all commands,
    // LuaCommandWrapper destructors should clean up their Lua registry entries.
}

std::string LuaPlugin::getName() const {
    lua_State* L = lua_manager_->state();
    if (L && hasLuaFunction("get_plugin_name")) {
        lua_getglobal(L, "get_plugin_name");
        if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
            if (lua_isstring(L, -1)) {
                std::string name_from_lua = lua_tostring(L, -1);
                lua_pop(L, 1);
                if (!name_from_lua.empty()) return name_from_lua;
            } else {
                lua_pop(L, 1);
            }
        } else {
            lua_pop(L, 1); // Pop error message
        }
    }
    return plugin_name_;
}

bool LuaPlugin::hasLuaFunction(const char* funcName) const {
    if (!lua_manager_ || !lua_manager_->state()) return false;
    lua_State* L = lua_manager_->state();
    lua_getglobal(L, funcName);
    bool isFunction = lua_isfunction(L, -1);
    lua_pop(L, 1);
    return isFunction;
}

void LuaPlugin::callLuaPluginFunction(const char* funcName, CommandRegistry* registry) {
    if (!lua_manager_ || !lua_manager_->state()) return;
    lua_State* L = lua_manager_->state();
    lua_getglobal(L, funcName);
    if (lua_isfunction(L, -1)) {
        // If passing registry to Lua: lua_pushlightuserdata(L, registry); lua_pcall(L, 1, 0, 0)
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            std::cerr << "LuaPlugin: Error calling function '" << funcName << "' in plugin " << getName() << ": " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1); // Pop non-function global
    }
}

void LuaPlugin::initialize(CommandRegistry* registry) {
    command_registry_ptr_ = registry;
    if (hasLuaFunction("initialize")) {
        callLuaPluginFunction("initialize", registry);
    }
}

void LuaPlugin::shutdown(CommandRegistry* registry) {
    command_registry_ptr_ = registry; // Ensure registry is available for Lua's shutdown
    if (hasLuaFunction("shutdown")) {
        callLuaPluginFunction("shutdown", registry);
    }
    // Commands should ideally be unregistered by the Lua script's shutdown function.
    // LuaCommandWrapper destructors will also try to clean up their entries from Lua registry.
    command_registry_ptr_ = nullptr;
}

// Implementation of C functions exposed to Lua
static int lua_neurodeck_register_command(lua_State* L) {
    // Upvalue index 1 has the LuaPlugin*
    LuaPlugin* plugin = static_cast<LuaPlugin*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!plugin || !plugin->command_registry_ptr_ || !plugin->getLuaManager() || !plugin->getLuaManager()->state()) {
        return luaL_error(L, "neurodeck_register_command: Internal plugin context error.");
    }
    CommandRegistry* registry = plugin->command_registry_ptr_;
    lua_State* plugin_L = plugin->getLuaManager()->state(); // This is L, but good for clarity

    const char* name = luaL_checkstring(L, 1);
    const char* description = luaL_checkstring(L, 2);
    if (!lua_isfunction(L, 3)) {
        return luaL_error(L, "neurodeck_register_command: Arg 3 (run_function) must be a Lua function.");
    }

    std::string lua_func_key = plugin->getName() + "::" + name;
    lua_pushvalue(L, 3); // Duplicate the Lua function
    lua_setfield(plugin_L, LUA_REGISTRYINDEX, lua_func_key.c_str()); // Store in plugin_L's registry

    auto wrapper = std::make_unique<LuaCommandWrapper>(name, description, plugin_L, lua_func_key);
    registry->register_command(std::move(wrapper));
    return 0;
}

static int lua_neurodeck_unregister_command(lua_State* L) {
    LuaPlugin* plugin = static_cast<LuaPlugin*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!plugin || !plugin->command_registry_ptr_ || !plugin->getLuaManager() || !plugin->getLuaManager()->state()) {
        return luaL_error(L, "neurodeck_unregister_command: Internal plugin context error.");
    }
    CommandRegistry* registry = plugin->command_registry_ptr_;
    lua_State* plugin_L = plugin->getLuaManager()->state();

    const char* name = luaL_checkstring(L, 1);
    std::string lua_func_key = plugin->getName() + "::" + name;

    // Remove Lua function from registry
    lua_pushnil(plugin_L);
    lua_setfield(plugin_L, LUA_REGISTRYINDEX, lua_func_key.c_str());

    registry->unregister_command(name); // This will delete the LuaCommandWrapper,
                                       // whose destructor might also try to clean the registry key. It's fine.
    return 0;
}

} // namespace Neurodeck
