#define LUA_LIB

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};

#include <boost/locale/encoding.hpp>
#include <string>

std::string utf8_to_gbk(const std::string & str)
{
    return boost::locale::conv::between(str, "GBK", "UTF-8");
}

std::string gbk_to_utf8(const std::string & str)
{
    return boost::locale::conv::between(str, "UTF-8", "GBK");
}

int lua_utf8_to_gbk(lua_State* L)
{
    std::string src = luaL_checkstring(L, 1);
    std::string dst = utf8_to_gbk(src);
    lua_pushstring(L, dst.c_str());
    return 1;
}

int lua_gbk_to_utf8(lua_State* L)
{
    std::string src = luaL_checkstring(L, 1);
    std::string dst = gbk_to_utf8(src);
    lua_pushstring(L, dst.c_str());
    return 1;
}

extern "C" 
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