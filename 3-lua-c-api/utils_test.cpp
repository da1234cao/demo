#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <iostream>
#include <string>

int main()
{
    lua_State *L = luaL_newstate();

    luaL_openlibs(L);

    luaL_dofile(L, "./utils.lua");

    lua_getglobal(L, "version");
    std::string version = lua_tostring(L, -1);
    std::cout << version << std::endl;
    
    lua_getglobal(L, "sub");
    lua_pushnumber(L, 10);
    lua_pushnumber(L, 2);
    lua_call(L, 2, 1);
    int sub_result = lua_tonumber(L, -1);
    std::cout << sub_result << std::endl;

    return 0;
}