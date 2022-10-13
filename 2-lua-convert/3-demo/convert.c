#define LUA_LIB

//  g++ .\convert.c -o .\convert.dll -I ..\..\src\ -L ..\..\src\  -llua53 -mdll  -DLUA_BUILD_AS_DLL

#ifdef __WIN32__
#include <windows.h>
#else
#include <iconv.h>
#endif

#include <lua.hpp>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>

#ifdef __WIN32__
extern "C" int utf8_to_gb(lua_State* L)
{
    const char *src = luaL_checkstring(L, 1);

    int ret = -1;
    WCHAR* strA = NULL;
    char *dst = NULL;
    int i = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
    if (i <= 0) {
        goto end;
    }
    strA = (WCHAR*)malloc(i * 2);
    MultiByteToWideChar(CP_UTF8, 0, src, -1, strA, i);
    i = WideCharToMultiByte(CP_ACP, 0, strA, -1, NULL, 0, NULL, NULL);
    dst = (char*)malloc(i+1);
    memset(dst, 0, i+1);  
    ret = WideCharToMultiByte(CP_ACP, 0, strA, -1, dst, i, NULL, NULL);

end:    
    lua_pushstring(L, dst);
    lua_pushnumber(L, ret);
    if(strA) {free(strA);}
    if(dst) {free(dst);}
    return 2;
}

extern "C" int gb_to_utf8(lua_State* L)
{
    const char *src = luaL_checkstring(L, 1);
 
    int ret = -1;
    WCHAR* strA = NULL;
    char *dst = NULL;
    int i= MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
    if (i <= 0) {
        goto end;
    }
    strA = (WCHAR*)malloc(i * 2);
    MultiByteToWideChar(CP_ACP, 0, src, -1, strA, i);
    i = WideCharToMultiByte(CP_UTF8, 0, strA, -1, NULL, 0, NULL, NULL);
    dst = (char*)malloc(i+1);
    memset(dst, 0, i+1);
    ret = WideCharToMultiByte(CP_UTF8, 0, strA, -1, dst, i, NULL, NULL);

end:
    lua_pushstring(L, dst);
    lua_pushnumber(L, ret);
    if(strA) {free(strA);}
    if(dst) {free(dst);}
    return 2;
}
#else
extern "C" int utf8_to_gb(lua_State* L)
{
    char *inbuf = luaL_checkstring(L, 1);

    int ret = -1;
    size_t inlen = strlen(inbuf);
    size_t outlen = len;
    char *outbuf = (char*)malloc(len);
    memset(outbuf, 0, len);
    iconv_t = iconv_open("GBK", "UTF-8");
    if (cd != (iconv_t)-1) {
        if (iconv(cd, &inbuf, &inlen, &outbuf, &outlen) != 0) {
           outlen = -1;
           goto end;
        }
    }

end:
    ret = outlen;
    lua_pushstring(L, outbuf);
    lua_pushnumber(L, ret);
    if (cd != (iconv_t)-1) {
        iconv_close(cd);
    }
    free(outbuf); 
    return 2;
}

extern "C" int gb_to_utf8(lua_State* L)
{
    char *inbuf = luaL_checkstring(L, 1);

    int ret = -1;
    size_t inlen = strlen(inbuf) + 1;
    size_t outlen = len*2;
    char *outbuf = (char*)malloc(outlen);
    memset(outbuf, 0, outlen);
    iconv_t cd = iconv_open("UTF-8", "GBK");
    if (cd != (iconv_t)-1) {
        if (iconv(cd, &inbuf, &inlen, &outbuf, &outlen) != 0) {
           outlen = -1;
           goto end;
        }
        iconv_close(cd);
    }

end:
    ret = outlen;
    lua_pushstring(L, outbuf);
    lua_pushnumber(L, ret);
    if (cd != (iconv_t)-1) {
        iconv_close(cd);
    }
    free(outbuf); 
    return 2;
}
#endif

extern "C" 
LUALIB_API int luaopen_convert(lua_State *L) {
    luaL_Reg libs[] = { 
        {"utf8_to_gb", utf8_to_gb},
        {"gb_to_utf8", gb_to_utf8},
        {NULL, NULL} 
    }; 
#if LUA_VERSION_NUM >= 502
    luaL_newlib(L, libs);
#else
    luaL_register(L, NULL, libs);
#endif
    return 1;
}