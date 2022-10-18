#pragma once

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <Windows.h>

#define WBUFF 2048
#define eq(s1,s2) (strcmp(s1,s2)==0)

static wchar_t wbuff[WBUFF];
LPWSTR wstring(const char *text);
int push_wstring(lua_State *L,LPCWSTR us);

int push_error_msg(lua_State *L, const char *msg);

int push_error(lua_State *L, int err);