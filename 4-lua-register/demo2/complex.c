#define LUA_BUILD_AS_DLL
#define LUA_LIB

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>

#define COMPLEX_MT "complex_mt"

//////// 数据结构

typedef struct {
  double real;
  double imag;
} complex;

/////// 数据结构的操作方法，存放在元表中

complex add(complex num_a, complex num_b)
{
  complex num_res;
  num_res.real = num_a.real + num_b.real;
  num_res.imag = num_a.imag + num_b.imag;
  return num_res;
}

complex sub(complex num_a, complex num_b)
{
  complex num_res;
  num_res.real = num_a.real - num_b.real;
  num_res.imag = num_a.imag - num_b.imag;
  return num_res;
}

int l_add(lua_State *L)
{
  complex *num_a = (complex *)luaL_checkudata(L,1,COMPLEX_MT);
  complex *num_b = (complex *)luaL_checkudata(L,2,COMPLEX_MT);
  complex num_res = add(*num_a, *num_b);
  complex *res = lua_newuserdata(L, sizeof(complex));
  res->real = num_res.real;
  res->imag = num_res.imag;
  luaL_getmetatable(L,COMPLEX_MT);
  lua_setmetatable(L,-2);
  return 1;
}

int l_sub(lua_State *L)
{
  complex *num_a = (complex *)luaL_checkudata(L,1,COMPLEX_MT);
  complex *num_b = (complex *)luaL_checkudata(L,2,COMPLEX_MT);
  complex num_res = sub(*num_a, *num_b);
  complex *res = lua_newuserdata(L, sizeof(complex));
  res->real = num_res.real;
  res->imag = num_res.imag;
  luaL_getmetatable(L,COMPLEX_MT);
  lua_setmetatable(L,-2);
  return 1;
}

int l_print(lua_State *L)
{
  complex *num = (complex *)luaL_checkudata(L,1,COMPLEX_MT);
  if(num->imag > 0) {
    printf("%f + %f", num->real, num->imag);
  } else if(num->imag == 0) {
    printf("%f", num->real);
  } else {
    printf("%f %f", num->real, num->imag);
  }
  return 0;
}

static const struct luaL_Reg complex_methods [] = {
  {"add", l_add},
  {"sub", l_sub},
  {"print", l_print},
  {NULL, NULL}
};

static void regist_complex_methods(lua_State *L)
{
  // 该库只需要一个complex元表。该元表可以绑定到不同的对象上。
  // 所以在加载库的时候，创建下元表即可
  luaL_newmetatable(L, COMPLEX_MT); //  为用户数据的元表创建一张新表，并添加到注册表中，最终将新表压栈
  luaL_setfuncs(L,complex_methods,0); // 将complex_methods中的方法，注册到栈顶的表中
  lua_pushvalue(L,-1); // 将上面创建的新表，做副本压栈
  lua_setfield(L,-2,"__index"); // COMPLEX_MT.__index = COMPLEX_MT(这样当一个表访问其没有的元素时，到其元表中查找)(同时副本出栈)
  lua_pop(L,1); // 元表出栈，此时该元表已经赋值过方法了。此后想访问该表，通过注册表即可
}

/////// 其他函数，非元表中方法

static int l_new(lua_State *L)
{
  double real = lua_tonumber(L, 1);
  double imag = lua_tonumber(L, 2);
  complex *res = lua_newuserdata(L, sizeof(complex)); // 分配一块指定大小的内存块， 把内存块地址作为一个完全用户数据压栈， 并返回这个地址。 宿主程序(lua)可以随意使用这块内存
  res->real = real;
  res->imag = imag;
  luaL_getmetatable(L,COMPLEX_MT); // 从注册表中获取元表
  lua_setmetatable(L,-2); // 给创建的用户数据，绑定上元表，并将表弹出栈
  return 1; // 表示C在虚拟栈中，存放了一个返回值，即上面创建的userdata的地址
}

static const luaL_Reg complex_func[] = {
  {"new", l_new},
  {NULL, NULL}
};

LUALIB_API int luaopen_complex(lua_State *L) {
  luaL_newlib(L, complex_func);
  regist_complex_methods(L);
  return 1;
}