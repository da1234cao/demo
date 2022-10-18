## 前言

这篇博客，我估计写不好。一方面是内容挺绕，一方面是我没有看过书，是照葫芦画瓢写代码。

前置要求：
* [Lua调用C代码](https://blog.csdn.net/sinat_38816924/article/details/127311158)
* [lua中表与元表](https://blog.csdn.net/sinat_38816924/article/details/127380757)
* [Lua操作C语言用户自定义类型数据Userdata](https://blog.csdn.net/DeliaPu/article/details/118115256)

上面第三个链接的内容是，C语言自定义数据类型，在Lua中使用userdata表示。Lua调用C中的函数，操作userdata。这样写也挺好，但是分割了userdata和函数。

我们知道，lua有元表。那能否**将C中的函数，注册到lua的元表中，再将元表绑定到userdata上**呢？

注： 
* 本文完整代码见仓库。
* 《Lua程序设计》第四版 第三十一章 C语言中的用户自定义类型，应该是比较详细的介绍(我看了下示例代码，文字没看)。

---

## 给userdata绑定元表

回头来看写的代码，逻辑上(非执行顺序上)基本上是这样的思路。
* 第一步：提供一个函数来创建userdata。（类似与面向对象中的new）
* 第二步：在lua加载C动态库的时候创建元表，在元表上注册可以操作userdata的方法。（因为这个元表只需要一份，所以在加载动态库的时候创建，合适不过了）
* 第三步：在第一步创建userdata的时候，将第二步创建的元表附加上。(这样的好处是，不同的userdata对象，用相同的元表。即不同的对象，有相同的方法。)

示例代码如下。

```c
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
```

编译代码如下。

```cmake
cmake_minimum_required(VERSION 3.11)

project(complex C)

# lua path
set(LUA_HEADER_PAHT C:/lua64/include)
set(LUA_LIB_PAHT C:/lua64)
set(LUA_LIB_NAME "lua53")
include_directories(${LUA_HEADER_PAHT})
link_directories(${LUA_LIB_PAHT})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/complex.c)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${LUA_LIB_NAME})
```

测试代码如下。

```lua
local complex = require "complex"

local num_a = complex.new(1,2)
local num_b = complex.new(3,4)
local num_c = num_a:add(num_b)
num_c:print()

print() -- 换行

local num_d = num_b:sub(num_a)
num_d:print()
```

输出如下。

```lua
C:\lua64\lua.exe .\test.lua
4.000000 + 6.000000
2.000000 + 2.000000
```

---

## userdata绑定元表应用 -- 查看注册表

代码不能凭空写出，一般是有参考的。上面代码结构参考自：[stevedonovan/winapi](https://github.com/stevedonovan/winapi/blob/master/winapi.c)

我将其中使用lua查看[windows注册表信息](https://learn.microsoft.com/zh-cn/troubleshoot/windows-server/performance/windows-registry-advanced-users)的接口扣了出来。

这里不展开介绍，只是一个应用，自行见仓库demo3。


