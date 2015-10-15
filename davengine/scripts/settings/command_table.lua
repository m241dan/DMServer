command_table = {}

-- name, path, level --

-- account commands --
account_commands = { { "create a character", "../scripts/commands/charcreate.lua", 0 },
                     { "play a character", "../scripts/commands/playchar.lua", 0 }
                   }
-- mobile commands --
mobile_commands = { { "test", "../scripts/commands/test.lua", 0 } }


-- assigning tables to appropriate state indexes --
command_table[1] = account_commands
command_table[9] = mobile_commands



