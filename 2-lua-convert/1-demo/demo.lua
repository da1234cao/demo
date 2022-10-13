-- 文件保存为utf8编码
-- 由于win下默认使用的是gtk编码。所以会检查不到文件的存在
-- 所以，需要将文件名进行编码转换：utf8 -> gdk

local gbk = require 'gbk'

file_name = "D:\\我是中文路径.txt"

file_name = gbk.fromutf8(file_name)

function file_exist_check(file_path)
  local result = false
  local f = io.open(file_path, "rb")
  if (f ~= nil) then 
    result = true
  end
  return result
end

tmp = file_exist_check(file_name)
if (tmp == true) then 
  print("file exist!")
else 
  print("file not exist!")
end