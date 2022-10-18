#include "register.h"

typedef struct {
  HKEY key;
} Regkey;

#define Regkey_MT "Regkey"

Regkey *Regkey_arg(lua_State *L,int idx) {
  Regkey *this = (Regkey *)luaL_checkudata(L,idx,Regkey_MT);
  luaL_argcheck(L, this != NULL, idx, "Regkey expected");
  return this;
}


static int l_Regkey_get_value(lua_State *L) {
  Regkey *this = Regkey_arg(L,1);
  const char *name = luaL_optlstring(L,2,"",NULL);
  DWORD type;
  DWORD size = sizeof(wbuff);
  void *data = wbuff;
  int res = RegQueryValueExW(this->key,wstring(name),0,&type,data,&size);
  if (res != ERROR_SUCCESS) {
    return push_error(L, res);
  }
  if (type == REG_BINARY) {
    lua_pushlstring(L,(const char *)data,size);
  } else if (type == REG_EXPAND_SZ || type == REG_SZ) {
    push_wstring(L,wbuff);
  } else {
    lua_pushnumber(L,*(unsigned long *)data);
  }
  lua_pushinteger(L,type);
  return 2;
}

static const struct luaL_Reg Regkey_methods [] = {
  {"get_value",l_Regkey_get_value},
  {NULL, NULL}  /* sentinel */
};

static void Regkey_register(lua_State *L) {
  luaL_newmetatable(L,Regkey_MT);
  luaL_setfuncs(L,Regkey_methods,0);
  lua_pushvalue(L,-1); // push Regkey
  lua_setfield(L,-2,"__index"); // Regkey.__index = Regkey
  lua_pop(L,1);
}


static HKEY predefined_keys(LPCSTR key) {
  #define check(predef) if (eq(key,#predef)) return predef;
  check(HKEY_CLASSES_ROOT);
  check(HKEY_CURRENT_CONFIG);
  check(HKEY_CURRENT_USER);
  check(HKEY_LOCAL_MACHINE);
  check(HKEY_USERS);
  #undef check
  return NULL;
}


HKEY split_registry_key(LPCSTR path, char *keypath) {
  char key[MAX_KEY];
  LPCSTR slash = strchr(path,SLASH);
  int i = (int)((DWORD_PTR)slash - (DWORD_PTR)path);
  strncpy(key,path,i);
  key[i] = '\0';
  strcpy(keypath,path+i+1);
  return predefined_keys(key);
}


static int l_open_reg_key(lua_State *L)
{
    const char *path = luaL_checklstring(L, 1, NULL);
    int writeable = lua_toboolean(L, 2);
    HKEY hKey;
    char subkey[MAX_PATH];
    DWORD access;
    hKey = split_registry_key(path, subkey);
    if(hKey == NULL) {
      return push_error_msg(L,"unrecognized registry key");
    }
    access = writeable ? KEY_ALL_ACCESS : (KEY_READ | KEY_ENUMERATE_SUB_KEYS);
    LSTATUS ret = RegOpenKeyExW(hKey,wstring(subkey),0,access,&hKey);
    if (ret == ERROR_SUCCESS) {
      Regkey *this = (Regkey *)lua_newuserdata(L,sizeof(Regkey));
      luaL_getmetatable(L,Regkey_MT);
      lua_setmetatable(L,-2);
      this->key = hKey;
      return 1;
    } else {
      return push_error(L, ret);
    }
}

static const luaL_Reg reg_func[] = {
  {"open_reg_key", l_open_reg_key},
  {NULL, NULL}
};

LUALIB_API int luaopen_register(lua_State *L) {
  luaL_newlib(L, reg_func);
  Regkey_register(L);
  return 1;
}