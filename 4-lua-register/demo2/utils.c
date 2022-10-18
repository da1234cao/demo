#include "utils.h"

LPWSTR wstring_buff(LPCSTR text, LPWSTR wbuf, int bufsz) {
  int res = MultiByteToWideChar(
    CP_UTF8, 0,
    text,-1,
    wbuf,bufsz);
  if (res != 0) {
    return wbuf;
  } else {
    return NULL; // how to indicate error, hm??
  }
}

LPWSTR wstring(const char *text) {
  memset(wbuff, 0, sizeof(wbuff));
  return wstring_buff(text,wbuff,sizeof(wbuff));
}

int push_wstring_l(lua_State *L, LPCWSTR us, int len) {
  int osz = 3*len;
  char *obuff = malloc(osz);
  int res = WideCharToMultiByte(
    CP_UTF8, 0,
    us,len,
    obuff,osz,
    NULL,NULL);
  if (res == 0) {
    free(obuff);
    return push_error(L, res);
  } else {
    lua_pushlstring(L,obuff,res);
    free(obuff);
    return 1;
  }
}

int push_wstring(lua_State *L,LPCWSTR us) {
  int len = wcslen(us);
  return push_wstring_l(L,us,len);
}

const char *last_error(int err) {
  static char errbuff[256];
  memset(errbuff, 0, sizeof(errbuff));
  int sz;
  if (err == 0) {
    err = GetLastError();
  }
  sz = FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,err,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    errbuff, 256, NULL
  );
  errbuff[sz-2] = '\0'; // strip the \r\n
  return errbuff;
}

int push_error_msg(lua_State *L, const char *msg) {
  lua_pushnil(L);
  lua_pushstring(L,msg);
  return 2;
}

int push_error(lua_State *L, int err) {
  return push_error_msg(L,last_error(err));
}