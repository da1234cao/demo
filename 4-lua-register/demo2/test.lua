local reg = require "register"

-- local key = require.open_reg_key("HKEY_CURRENT_USER\\Environment"
local key = reg.open_reg_key [[HKEY_CURRENT_USER\Environment]]
print(key.get_value(key,"PATH"))
