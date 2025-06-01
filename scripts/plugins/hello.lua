-- scripts/plugins/hello.lua

local plugin_name = "HelloLuaPlugin"

-- Optional: Define a function to explicitly provide the plugin name
function get_plugin_name()
    return plugin_name
end

local function hello_command_run(args_table)
    -- args_table[1] is the command name ("hello_lua")
    if #args_table > 1 then
        print("HelloLuaPlugin: The 'hello_lua' command does not take any arguments.")
    else
        print("Hello from Lua plugin!")
    end
end

function initialize()
    -- print(plugin_name .. ": Initializing...")
    neurodeck_register_command("hello_lua", "Prints a greeting message from the hello.lua plugin.", hello_command_run)
    -- print(plugin_name .. ": 'hello_lua' command registered.")
end

function shutdown()
    -- print(plugin_name .. ": Shutting down...")
    neurodeck_unregister_command("hello_lua")
    -- print(plugin_name .. ": 'hello_lua' command unregistered.")
end

-- print(plugin_name .. " script loaded by Lua interpreter.")
