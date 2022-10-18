
local complex_metatb = {}
complex_metatb.__index = complex_metatb

complex_metatb.__add = function(compx1, compx2)
  local res = {real=0, imag=0}
  res.real = compx1.real + compx2.real
  res.imag = compx1.imag + compx2.imag
  print(res)
  return res
end

complex_metatb.__tostring = function(complx)
  local str
  if(complx.imag > 0) then
    str = string.format("%f + %f", complx.real, complx.imag)
  elseif(complx.imag == 0) then
    str = string.format("%f", complx.real)
  else
    str = string.format("%f %f", complx.real, complx.imag)
  end
  return str
end

function complex_metatb.print(complx)
  local str
  if(complx.imag > 0) then
    str = string.format("%f + %f", complx.real, complx.imag)
  elseif(complx.imag == 0) then
    str = string.format("%f", complx.real)
  else
    str = string.format("%f %f", complx.real, complx.imag)
  end
  print(str)
end

local num_a = {real=1, imag=2}
local num_b = {real=3, imag=4}
setmetatable(num_a, complex_metatb)
setmetatable(num_b, complex_metatb)

local res = num_a + num_b
print(res)
setmetatable(res, complex_metatb)
-- print(res)
res:print() -- res.print(res)

