## 准备工作

### windows下环境准备

方式一：下载[lua-5.3.6源码](https://www.lua.org/ftp/)、[安装MinGW](http://c.biancheng.net/view/8077.html)、使用命令`mingw32-make mingw`进行编译。但是这种方式编译出来的是32位的。

方式二：或者，我们不编译，直接去下载lua的头文件，库和可执行文件。我们去官网下载[二进制lua](https://luabinaries.sourceforge.net/)。

方式三：自己编译。在win上没有make，虽然vs有namke.exe，但是直接使用它编译会报错。既然现在的代码可以用make编译，那编译可以使用cmake接管。自己将make转成CMakeLists.txt有点麻烦，网上有现成的[ squeek502/CMakeLists.txt](https://gist.github.com/squeek502/0706332efb18edd0601a4074762b0b9a)。(cmake的跨平台真香。**推荐这种方式**。)

---

### linux下环境准备

因为本次的lua代码，均在win下执行。所以不需要这一部分。完整性起见，补充了这一部分。

```shell
sudo apt install lua5.3
# sudo apt install liblua5.3-0
sudo apt install liblua5.3-dev

lua -v
Lua 5.3.3  Copyright (C) 1994-2016 Lua.org, PUC-Rio


# liblua库的头文件和库位置我们需要知道下
➜  lib sudo dpkg -L liblua5.3-dev:amd64 
...
/usr/include/lua5.3
/usr/include/lua5.3/lauxlib.h
...
/usr/lib/x86_64-linux-gnu/liblua5.3-c++.a
/usr/lib/x86_64-linux-gnu/liblua5.3.a
...
```

---

## 前言

lua默认使用的是utf8编码。

某些时候，需要使用lua进行gbk和utf8之间的转换。

比如，当想检查win下某个文件是否存在的时候。需要将文件名字符串转换成gbk编码。

我们可以在D盘下面，创建一个包含中文路径的文件，测试运行下面程序，进行测试。

```lua
-- 文件保存为utf8编码
-- 由于win下默认使用的是gtk编码。所以会检查不到文件的存在
-- 所以，需要将文件名进行编码转换：utf8 -> gdk

file_name = "D:\\我是中文路径.txt"

-- file_name = gbk.fromutf8(file_name) --have to implement the gbk.fromutf8 function by yourself

function file_exist_check(file_path)
  local result = false
  local f = io.open(file_path, "rb")
  if (f ~= nil) then 
    result = true
  end
  return result
end

tmp = file_exist_check(file_name)
if (tmp == true) then 
  print("file exist!")
else 
  print("file not exist!")
end
```

lua本身没有提供编码转换的[标准库](https://cloudwu.github.io/lua53doc/manual.html#6.1)。所以，我们只好通过lua调用C代码，来实现编码转换。

---

### luagbk

这位大佬提供了一个解决方案：[starwing/luagbk](https://github.com/starwing/luagbk)

优点：编码转换不依赖外部API

缺点：优点也是缺点。我不了解编码转换的内部细节，所以我不信任仓库中的代码。代码如果有意外的bug，我也不会修复。

虽然在实际中，我不会使用这个仓库的代码。但我使用demo测试了下，确实有效。demo可见`2-demo`目录。

```shell
mkdir 1-demo && cd 1-demo
git clone git@github.com:starwing/luagbk.git
cd luagbk
gcc lgbk.c -o gbk.dll -I ..\..\lua\src -L ..\..\lua\src -llua53 -s -mdll -O2 -DLUA_BUILD_AS_DLL
```

我们将生成的gbk.dll复制到和demo相同的目录下，测试，编码转换没有问题。

```shell
..\lua\src\lua.exe .\demo.lua
file exist!
```

---

### lua调用C代码进行编码转换

上面的仓库代码，我看不懂，但是我可以写一个编码转换的C函数，编译成动态库，然后让lua调用。

C编码转换的代码修改自：[lytsing/gbk-utf8](https://github.com/lytsing/gbk-utf8)

代码结构见demo2。由于后续想把这个文档作为博客，所以这里也会粘贴完整代码。

编码转换的头文件定义如下。

```c
#ifndef CONVERT_H
#define CONVERT_H

#ifdef __WIN32__
#include <windows.h>
#else
#include <iconv.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif

// 禁止输入空字符串；
// 返回值小于等于0，为出错；大于零，为转换后的字符个数

int utf8_to_gbk(const char* src, char* dst, int len);

int gbk_to_utf8(const char* src, char* dst, int len);

#ifdef __cplusplus
}
#endif

#endif  // end of CONVERT_H
```

头文件中函数的实现。

```c
#include "convert.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __WIN32__
int utf8_to_gbk(const char* src, char* dst, int len)
{
    int ret = 0;
    WCHAR* strA;
    int i= MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
    if (i <= 0) {
        return i;
    }
    strA = (WCHAR*)malloc(i * 2);
    MultiByteToWideChar(CP_UTF8, 0, src, -1, strA, i);
    i = WideCharToMultiByte(CP_ACP, 0, strA, -1, NULL, 0, NULL, NULL);
    if (len > i) {
        ret = WideCharToMultiByte(CP_ACP, 0, strA, -1, dst, i, NULL, NULL);
        dst[i] = 0;
    }

    free(strA);
    return ret;
}

int gbk_to_utf8(const char* src, char* dst, int len)
{
    int ret = 0;
    WCHAR* strA;
    int i= MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
    if (i <= 0) {
        return i;
    }
    strA = (WCHAR*)malloc(i * 2);
    MultiByteToWideChar(CP_ACP, 0, src, -1, strA, i);
    i = WideCharToMultiByte(CP_UTF8, 0, strA, -1, NULL, 0, NULL, NULL);
    if (len >= i) {
        ret = WideCharToMultiByte(CP_UTF8, 0, strA, -1, dst, i, NULL, NULL);
        dst[i] = 0;
    }
    free(strA);
    return ret;
}
#else   //Linux
// starkwong: In iconv implementations, inlen and outlen should be type of size_t not uint, which is different in length on Mac
int utf8_to_gbk(const char* src, char* dst, int len)
{
    int ret = 0;
    size_t inlen = strlen(src) + 1;
    size_t outlen = len;

    // duanqn: The iconv function in Linux requires non-const char *
    // So we need to copy the source string
    char* inbuf = (char *)malloc(len);
    char* inbuf_hold = inbuf;   // iconv may change the address of inbuf
                                // so we use another pointer to keep the address
    memcpy(inbuf, src, len);

    char* outbuf = dst;
    iconv_t cd;
    cd = iconv_open("GBK", "UTF-8");
    if (cd != (iconv_t)-1) {
        if (iconv(cd, &inbuf, &inlen, &outbuf, &outlen) != 0) {
            free(inbuf_hold);
            return 0;
        }
        iconv_close(cd);
    }
    ret = outlen;
    free(inbuf_hold);
    return ret;
}

void gbk_to_utf8(const char* src, char* dst, int len)
{
    int ret = 0;
    size_t inlen = strlen(src) + 1;
    size_t outlen = len;

    // duanqn: The iconv function in Linux requires non-const char *
    // So we need to copy the source string
    char* inbuf = (char *)malloc(len);
    char* inbuf_hold = inbuf;   // iconv may change the address of inbuf
                                // so we use another pointer to keep the address
    memcpy(inbuf, src, len);

    char* outbuf2 = NULL;
    char* outbuf = dst;
    iconv_t cd;

    // starkwong: if src==dst, the string will become invalid during conversion since UTF-8 is 3 chars in Chinese but GBK is mostly 2 chars
    if (src == dst) {
        outbuf2 = (char*)malloc(len);
        memset(outbuf2, 0, len);
        outbuf = outbuf2;
    }

    cd = iconv_open("UTF-8", "GBK");
    if (cd != (iconv_t)-1) {
        if (iconv(cd, &inbuf, &inlen, &outbuf, &outlen) != 0) {
            free(inbuf_hold);
            if(src == dst) {
                free(outbuf2);
            }
            return 0;
        }else if (outbuf2 != NULL) { // 函数执行成功，且dst与src不相同，需要拷贝
            strcpy(dst, outbuf2);
            free(outbuf2);
        }
        iconv_close(cd);
    }
    ret = outlen;
    free(inbuf_hold);  
    return 0;
}
#endif

#ifdef __cplusplus
}
#endif
```

lua调用C的代码如下。

```lua
-- 文件保存为utf8编码
-- 由于win下默认使用的是gtk编码。所以会检查不到文件的存在
-- 所以，需要将文件名进行编码转换：utf8 -> gdk

local convert = require 'convert'

file_name = "D:\\我是中文路径.txt"
ret_val,ret_str = convert.utf8_to_gbk(file_name)
print(ret_val)
print(ret_str)

function file_exist_check(file_path)
  local result = false
  local f = io.open(file_path, "rb")
  if (f ~= nil) then 
    result = true
  end
  return result
end

tmp = file_exist_check(ret_str)
if (tmp == true) then 
  print("file exist!")
else 
  print("file not exist!")
end
```

win下使用mingw的编译命令如下。

```shell
g++ lconvert.c convert.c -o utf8.dll -I ..\lua\src\ -L ..\lua\src\  -llua53 -mdll  -DLUA_BUILD_AS_DLL 
```

---

### lua调用C/C++代码进行编码转换

我不喜欢C语言。C语言写起来，总是要考虑内存。我之前想要有一个C实现的类似C++ map的库。没找见，只找见了[GitHub - rxi/map: A type-safe hash map implementation for C](https://github.com/rxi/map)。有C++，再写C，简直折磨。

C++的标准库没有提供编码转换，boost库提供了。那就非常nice了。

`convert.cpp`实现编码转换。

```cpp
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
```

通过cmake编译生成动态库，CMakeLists.txt如下。

```cmake
cmake_minimum_required(VERSION 3.11)

project(convert)

ADD_DEFINITIONS(-DLUA_BUILD_AS_DLL)

set(LUA_HEADER_PAHT ${CMAKE_CURRENT_SOURCE_DIR}/../lua64/src)
set(LUA_LIB_PAHT ${CMAKE_CURRENT_SOURCE_DIR}/../lua64)
set(LUA_LIB_NAME "lua53")
include_directories(${LUA_HEADER_PAHT})
link_directories(${LUA_LIB_PAHT})

# boost
set(Boost_USE_STATIC_LIBS
    ON
    CACHE BOOL "Set using static boost library")
find_package(
  Boost REQUIRED
  COMPONENTS iostreams
             locale)
if(WIN32)
  link_directories(${PROJECT_SOURCE_DIR} $ENV{BOOST_ROOT}/lib)
endif(WIN32)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/convert.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${LUA_LIB_NAME})
```

测试的lua代码如下。

```lua
-- 文件保存为utf8编码
-- 由于win下默认使用的是gtk编码。所以会检查不到文件的存在
-- 所以，需要将文件名进行编码转换：utf8 -> gdk

local convert = require 'convert'

file_name = "D:\\我是中文路径.txt"
ret_str = convert.utf8_to_gbk(file_name)
print(ret_str)

function file_exist_check(file_path)
  local result = false
  local f = io.open(file_path, "rb")
  if (f ~= nil) then 
    result = true
  end
  return result
end

tmp = file_exist_check(ret_str)
if (tmp == true) then 
  print("file exist!")
else 
  print("file not exist!")
end
```

这代码量少了很多。lua调用C，C再调用C++，这样来的就舒服很多。
