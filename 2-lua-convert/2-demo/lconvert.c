//  gcc lutf8.c utf8.c -o .\utf8.dll -I ..\lua\src\ -L ..\lua\src\  -llua53 -mdll  -DLUA_BUILD_AS_DLL
// #define LUA_BUILD_AS_DLL
#define LUA_LIB

#include "convert.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#ifdef __cplusplus
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
#endif
int lua_utf8_to_gbk(lua_State* L)
{
    const char *src = luaL_checkstring(L, 1);
    int outlen = strlen(src);
    char *dst = (char*)malloc(outlen);
    
    int ret = utf8_to_gbk(src, dst, outlen);

    lua_pushnumber(L, ret);
    lua_pushstring(L, dst);
    return 2;
}

#ifdef __cplusplus
extern "C"
#endif
int lua_gbk_to_utf8(lua_State* L)
{
    const char *src = luaL_checkstring(L, 1);
    int outlen = strlen(src);
    char *dst = (char*)malloc(outlen);
    
    int ret = gbk_to_utf8(src, dst, outlen);
    
    lua_pushnumber(L, ret);
    lua_pushstring(L, dst);
    return 2;
}

#ifdef __cplusplus
extern "C"
#endif
LUALIB_API int luaopen_convert(lua_State *L) {
    luaL_Reg libs[] = { 
        {"utf8_to_gbk", lua_utf8_to_gbk},
        {"gbk_to_utf8", lua_gbk_to_utf8},
        {NULL, NULL} 
    }; 
#if LUA_VERSION_NUM >= 502
    luaL_newlib(L, libs);
#else
    luaL_register(L, NULL, libs);
#endif
    return 1;
}




