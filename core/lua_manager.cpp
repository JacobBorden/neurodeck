#include "lua_manager.hpp"
#include <stdexcept> // For std::runtime_error
#include <iostream>  // For error reporting
#include <chrono>

namespace Neurodeck {

#ifdef LUA_FOUND_IN_CMAKE

static void hook(lua_State* L, lua_Debug* /*ar*/) {
    auto* start = static_cast<const std::chrono::steady_clock::time_point*>(
        lua_getextraspace(L));
    if (std::chrono::steady_clock::now() - *start > std::chrono::milliseconds(10))
        luaL_error(L, "Instruction limit exceeded");
}

static int l_shell_run(lua_State* L) {
    const char* cmd = luaL_checkstring(L, 1);
    // validate {timeout, capture} table if provided...
    int ret = system(cmd);  // placeholder; replace with posix fork/exec & capture
    lua_pushinteger(L, ret);
    // TODO: emit audit log here
    return 1;
}

LuaManager::LuaManager() : L_(lua_newstate(lua_Alloc, nullptr)) {
    if (!L_) throw std::runtime_error("lua_newstate failed");
    openWhitelistedLibs();
    // Attach timer origin in extra space
    auto* origin = new(lua_getextraspace(L_.get()))
        std::chrono::steady_clock::time_point(std::chrono::steady_clock::now());
    lua_sethook(L_.get(), hook, LUA_MASKCOUNT, 10000);
}

LuaManager::~LuaManager() = default;

void LuaManager::openWhitelistedLibs() {
    if (!L_) return;
    // Open *only* the libs we consider benign
    luaL_requiref(L_.get(), LUA_MATHLIBNAME, luaopen_math, 1);
    lua_pop(L_.get(), 1);
    luaL_requiref(L_.get(), LUA_TABLIBNAME,  luaopen_table, 1);
    lua_pop(L_.get(), 1);
    luaL_requiref(L_.get(), LUA_STRLIBNAME,  luaopen_string, 1);
    lua_pop(L_.get(), 1);
    // Do NOT open io, os, debug, package
    lua_newtable(L_.get());          // shell = {}
    lua_pushcfunction(L_.get(), l_shell_run);
    lua_setfield(L_.get(), -2, "run");
    lua_setglobal(L_.get(), "shell");
}

LuaManager::LuaManager(LuaManager&& other) noexcept : L_(std::move(other.L_)) {}

LuaManager& LuaManager::operator=(LuaManager&& other) noexcept {
    if (this != &other) {
        L_ = std::move(other.L_);
    }
    return *this;
}

bool LuaManager::executeString(const std::string& luaCode) {
    if (!L_) {
        std::cerr << "Lua state is not initialized." << std::endl;
        return false;
    }
    int error = luaL_dostring(L_.get(), luaCode.c_str());
    if (error) {
        std::cerr << "Error executing Lua string: " << lua_tostring(L_.get(), -1) << std::endl;
        lua_pop(L_.get(), 1);
        return false;
    }
    return true;
}

lua_State* LuaManager::state() noexcept {
    return L_.get();
}

#else // LUA_FOUND_IN_CMAKE is not defined

// Stub implementations if Lua is not found

LuaManager::LuaManager() : L_(nullptr) {
    // std::cout << "LuaManager (stub) initialized because Lua was not found." << std::endl;
}

LuaManager::~LuaManager() {
    // std::cout << "LuaManager (stub) destroyed." << std::endl;
}

void LuaManager::openWhitelistedLibs() {
    // std::cout << "LuaManager (stub): openStandardLibs called, but Lua is not available." << std::endl;
}

LuaManager::LuaManager(LuaManager&& other) noexcept : L_(nullptr) {
    // Moving a stub is a no-op in terms of Lua state
}

LuaManager& LuaManager::operator=(LuaManager&& other) noexcept {
    // Assigning a stub is a no-op
    return *this;
}

bool LuaManager::executeString(const std::string& luaCode) {
    std::cerr << "Lua is not available. Cannot execute string: " << luaCode << std::endl;
    return false;
}

lua_State* LuaManager::state() noexcept {
    return nullptr;
}

#endif // LUA_FOUND_IN_CMAKE

} // namespace Neurodeck
