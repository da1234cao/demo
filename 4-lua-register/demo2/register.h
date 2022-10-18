#define LUA_BUILD_AS_DLL
#define LUA_LIB

#include "utils.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <Windows.h>

#define MAX_KEY MAX_PATH
#define SLASH '\\'