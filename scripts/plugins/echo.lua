-- scripts/plugins/echo.lua

local plugin_name = "EchoLuaPlugin"

function get_plugin_name()
    return plugin_name
end

local function echo_command_run(args_table)
    -- args_table[1] is the command name ("echo_lua")
    local output = ""
    if #args_table > 1 then
        for i = 2, #args_table do
            output = output .. args_table[i]
            if i < #args_table then
                output = output .. " "
            end
        end
    end
    print(output)
end

function initialize()
    -- print(plugin_name .. ": Initializing...")
    neurodeck_register_command("echo_lua", "Prints the arguments passed to it. From echo.lua plugin.", echo_command_run)
    -- print(plugin_name .. ": 'echo_lua' command registered.")
end

function shutdown()
    -- print(plugin_name .. ": Shutting down...")
    neurodeck_unregister_command("echo_lua")
    -- print(plugin_name .. ": 'echo_lua' command unregistered.")
end

-- print(plugin_name .. " script loaded by Lua interpreter.")
