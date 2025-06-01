#include "lua_manager.hpp"
#include <stdexcept> // For std::runtime_error
#include <iostream>  // For error reporting, std::cout
#include <string>    // For std::string building
#include <chrono>

namespace Neurodeck {

#ifdef LUA_FOUND_IN_CMAKE

// Custom print function to replace Lua's default print
static int lua_custom_print(lua_State* L) {
    int n = lua_gettop(L); // Number of arguments
    std::string s = "";
    lua_getglobal(L, "tostring"); // Get Lua's tostring function
    for (int i = 1; i <= n; i++) {
        lua_pushvalue(L, -1); // Duplicate tostring function
        lua_pushvalue(L, i);  // Push argument
        lua_call(L, 1, 1);    // Call tostring(arg)
        const char* str = lua_tostring(L, -1);
        if (str == nullptr) { // Should not happen with tostring
            return luaL_error(L, "'tostring' must return a string to 'print'");
        }
        s += str;
        lua_pop(L, 1); // Remove result of tostring
        if (i < n) {
            s += "\t"; // Standard Lua print separates arguments with a tab
        }
    }
    lua_pop(L, 1); // Pop tostring function
    s += "\n"; // Standard Lua print adds a newline
    std::cout << s; // Output to C++ std::cout
    std::cout.flush(); // Ensure it's flushed immediately, important for tests
    return 0; // Number of results
}

// static void hook(lua_State* L, lua_Debug* /*ar*/) { // Entire hook mechanism removed
//     auto* start = static_cast<const std::chrono::steady_clock::time_point*>(
//         lua_getextraspace(L));
//     if (std::chrono::steady_clock::now() - *start > std::chrono::milliseconds(10))
//         luaL_error(L, "Instruction limit exceeded");
// }

static int l_shell_run(lua_State* L) {
    const char* cmd = luaL_checkstring(L, 1);
    // validate {timeout, capture} table if provided...
    int ret = system(cmd);  // placeholder; replace with posix fork/exec & capture
    lua_pushinteger(L, ret);
    // TODO: emit audit log here
    return 1;
}

LuaManager::LuaManager() : L_(luaL_newstate()) { // Use luaL_newstate for default allocator
    if (!L_) throw std::runtime_error("luaL_newstate failed");
    openWhitelistedLibs(); 
    // Hook mechanism fully removed
    // // Attach timer origin in extra space
    // // auto* origin = new(lua_getextraspace(L_.get()))
    // //     std::chrono::steady_clock::time_point(std::chrono::steady_clock::now());
    // // lua_sethook(L_.get(), hook, LUA_MASKCOUNT, 10000);
}

LuaManager::~LuaManager() = default;

void LuaManager::openWhitelistedLibs() {
    if (!L_) return;
    // Load all standard libraries. This includes base, io, math, string, table, os, package, etc.
    // This is simpler and more robust than loading them individually for now.
    // The PANIC error needs to be resolved first.
    luaL_openlibs(L_.get());

    // Override default Lua print function
    lua_pushcfunction(L_.get(), lua_custom_print);
    lua_setglobal(L_.get(), "print");

    // Now, register any custom functions or tables, like 'shell.run'
    lua_newtable(L_.get());          // Create 'shell' table
    lua_pushcfunction(L_.get(), l_shell_run); // Push our C function
    lua_setfield(L_.get(), -2, "run");        // Set shell.run = l_shell_run (pops function)
    lua_setglobal(L_.get(), "shell");         // Set global 'shell' to this table (pops table)
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
