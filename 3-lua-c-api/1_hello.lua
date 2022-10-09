function fact (n)
  if n == 0 then
    return 1
  else
    return n * fact(n-1)
  end
end

-- print("enter a number:")
-- print("before use io.write")
io.write("enter a number:")
a = io.read("*number")      -- read a number
-- io.write("after use io.write")

result = fact(a)
str = string.format("%d's factorial is: %d", a, result)
print(str)