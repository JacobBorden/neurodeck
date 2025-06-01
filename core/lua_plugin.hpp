#ifndef LUA_PLUGIN_HPP
#define LUA_PLUGIN_HPP

#include "plugin.hpp"
#include "lua_manager.hpp"
#include <string>
#include <memory>

namespace Neurodeck {
class CommandRegistry; // Forward declaration
}

namespace Neurodeck {

class LuaPlugin : public Plugin {
public:
    LuaPlugin(std::string script_path, std::unique_ptr<LuaManager> lua_manager);
    ~LuaPlugin() override;

    std::string getName() const override;
    void initialize(CommandRegistry* registry) override;
    void shutdown(CommandRegistry* registry) override;

    bool hasLuaFunction(const char* funcName) const;
    void callLuaPluginFunction(const char* funcName, CommandRegistry* registry);

    // Public member to be accessed by lua_CFunction (lua_register_command)
    CommandRegistry* command_registry_ptr_;
    LuaManager* getLuaManager() { return lua_manager_.get(); } // Getter for LuaManager

private:
    std::string script_path_;
    std::string plugin_name_; // Derived from script_path_ or from Lua's get_plugin_name()
    std::unique_ptr<LuaManager> lua_manager_;
};

} // namespace Neurodeck

#endif // LUA_PLUGIN_HPP
