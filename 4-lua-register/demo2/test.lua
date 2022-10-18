local complex = require "complex"

local num_a = complex.new(1,2)
local num_b = complex.new(3,4)
local num_c = num_a:add(num_b)
num_c:print()

print() -- 换行

local num_d = num_b:sub(num_a)
num_d:print()