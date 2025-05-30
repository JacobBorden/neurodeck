#include "lua_command.hpp"
#include <iostream>
#include <sstream> // Required for std::ostringstream

namespace Neurodeck {

LuaCommand::LuaCommand() : luaManager_(std::make_unique<LuaManager>()) {}

std::string LuaCommand::name() const {
    return "lua";
}

std::string LuaCommand::description() const {
    return "Executes Lua code. Usage: lua \"code_string\"";
}

#include <cstdio> // For fprintf

#include <cstdio> // For fprintf, fflush
#include <iostream> // For std::cerr, std::endl, std::flush

void LuaCommand::run(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        fprintf(stderr, "C_STDERR_TEST_NOARGS\n"); fflush(stderr); // Test C stderr capture
        std::cerr << "Usage: " << args[0] << " \"<lua_code_string>\"" << std::endl << std::flush;
        return;
    }

    // Concatenate all arguments after the command itself to form the Lua code string
    // This allows users to pass multi-part strings or strings with spaces
    // if their shell doesn't group them into a single argument.
    // However, the typical usage would be `lua "print('hello')"` where "print('hello')" is one argument.
    std::ostringstream luaCodeStream;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) {
            luaCodeStream << " "; // Add space between arguments if multiple are given
        }
        luaCodeStream << args[i];
    }
    std::string luaCode = luaCodeStream.str();

    if (luaCode.empty()) {
        fprintf(stderr, "C_STDERR_TEST_EMPTYCODE\n"); fflush(stderr); // Test C stderr capture
        std::cerr << "No Lua code provided." << std::endl << std::flush;
        return;
    }
    
    // Execute the Lua string
    // Output from Lua's print() will go to stdout.
    // Errors from LuaManager::executeString will go to stderr.
    bool success = luaManager_->executeString(luaCode);

    if (!success) {
        // Error message is already printed by LuaManager, 
        // but we can add a general failure message if desired.
        // std::cerr << "Lua execution failed." << std::endl; 
    }
}

} // namespace Neurodeck
