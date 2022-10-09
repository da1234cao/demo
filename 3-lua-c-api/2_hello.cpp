#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <iostream>

int main()
{
    lua_State *L = luaL_newstate();

    luaL_openlibs(L);

    lua_getglobal(L, "print");

    std::string str = "hello world";

    lua_pushstring(L, str.c_str());

    lua_call(L, 1, 0);

    return 0;
}