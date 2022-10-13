-- 文件保存为utf8编码
-- 由于win下默认使用的是gtk编码。所以会检查不到文件的存在
-- 所以，需要将文件名进行编码转换：utf8 -> gdk

local convert = require 'convert'

file_name = "D:\\我是中文路径.txt"
ret_str,ret_val = convert.utf8_to_gb(file_name)
print(ret_val)
print(ret_str)

function file_exist_check(file_path)
  local result = false
  local f = io.open(file_path, "rb")
  if (f ~= nil) then 
    result = true
  end
  return result
end

tmp = file_exist_check(ret_str)
if (tmp == true) then 
  print("file exist!")
else 
  print("file not exist!")
end