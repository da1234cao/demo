local tab = {}
tab[1] = 111
tab.second = "222"
tab["third"] = "333"
function tab.hello() 
  print("hello world")
end

for k, v in pairs(tab) do
  if(type(v) == "number") then
    print(v)
  elseif(type(v) == "string") then
    print(v)
  elseif(type(v) == "function") then
    v()
  end
end