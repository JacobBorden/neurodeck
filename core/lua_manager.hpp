#ifndef LUA_MANAGER_HPP
#define LUA_MANAGER_HPP

#include <stdexcept> // For std::runtime_error
#include <iostream>  // For std::cerr (error reporting)
#include <memory>    // For std::unique_ptr
#include <string>    // For std::string

// Forward declaration for lua_State to avoid including Lua headers globally if not needed
struct lua_State;

#ifdef LUA_FOUND_IN_CMAKE
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#endif // LUA_FOUND_IN_CMAKE

namespace Neurodeck
{

    class LuaManager
    {
    public:
        LuaManager();
        ~LuaManager();

        // Prevent copying and assignment
        LuaManager(const LuaManager &) = delete;
        LuaManager &operator=(const LuaManager &) = delete;

        // Allow moving
        LuaManager(LuaManager &&other) noexcept;
        LuaManager &operator=(LuaManager &&other) noexcept;

        // Basic method to execute a string
        bool executeString(const std::string &luaCode);

        // Getter for the lua_State (use with caution)
        lua_State *state() noexcept;

    private:
        struct LuaCloser
        {
            void operator()(lua_State *s) const noexcept
            {

#if defined(LUA_FOUND_IN_CMAKE)
                if (s)
                    lua_close(s);
#endif
            }
        };
        using LuaPtr = std::unique_ptr<lua_State, LuaCloser>;

        LuaPtr L_; // own the state
        void openWhitelistedLibs();
    };
} // namespace Neurodeck

#endif // LUA_MANAGER_HPP
