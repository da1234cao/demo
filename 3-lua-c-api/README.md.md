[TOC]

## 前言

略。

文中代码可以见[demo/3-lua-c-api at laboratory · da1234cao/demo · GitHub](https://github.com/da1234cao/demo/tree/laboratory/3-lua-c-api)

---

## 环境安装

相关连接：[lua官网-download](https://www.lua.org/download.html)、[Lua 环境安装 - Lua 5.3 基础教程 - 简单教程](https://www.twle.cn/l/yufei/lua53/lua-basic-environment.html)、[Installing Lua on Linux - Lua Quick Start Guide [Book]](https://www.oreilly.com/library/view/lua-quick-start/9781789343229/d972a6d4-a889-4d22-b350-13b5025d27e0.xhtml)

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

## lua编程

`lua`不是[函数式编程](https://zh.wikipedia.org/wiki/%E5%87%BD%E6%95%B0%E5%BC%8F%E7%BC%96%E7%A8%8B)语言，它和`C`差不多。基本上可以按照变量、表达式、语句、函数、标注库这套流程去了解。

比如：[《Lua 5.3 参考手册》](https://www.bookstack.cn/read/lua-5.3/README.md)、[《program in lua》中文版在线 lua5.0 - api过时](https://www.shouce.ren/api/lua/5/#)、[Lua 基础教程](https://www.twle.cn/l/yufei/lua53/lua-basic-index.html)

下面这个示例代码，修改自[Lua - 起点](https://www.shouce.ren/api/lua/5/#)

```lua
function fact (n)
  if n == 0 then
    return 1
  else
    return n * fact(n-1)
  end
end

-- print("enter a number:")
io.write("enter a number:")
a = io.read("*number")      -- read a number

result = fact(a)
str = string.format("%d's factorial is: %d", a, result)
print(str)
```

运行代码，输出如下：

```shell
lua hello.lua
enter a number:3
3 's factorial is: 6
```

PS: 上面使用`io.write`，而没有使用`print`，是因为`print`会默认换行，可参考[Lua print on the same line](https://stackoverflow.com/questions/7148678/lua-print-on-the-same-line)

---

## C++ 调用lua代码

相关连接：[《program in lua》第24章 C API纵览](https://www.shouce.ren/api/lua/5/#)、[C程序优雅地调用Lua?一篇博客带你全面了解lua_State](https://www.miaoerduo.com/2020/02/26/lua-state-tutorial/)、[Lua虚拟栈交互流程 - Bob的博客 | Bob Blog](https://chenanbao.github.io/2018/07/28/Lua%E8%99%9A%E6%8B%9F%E6%A0%88%E4%BA%A4%E4%BA%92%E6%B5%81%E7%A8%8B/)

通过上面三个连接，我们基本上可以知道如何在C++中调用lua代码。下面，我再画蛇添足的**搬运下上面链接中的内容**。

* 用于`C`和`lua`之间通信的虚拟栈的概念。

* `C`读写`lua`全局变量的函数。

* `C`调用`lua`函数的API。

---

### C++调用lua代码的准备工作

我们先来看一个C++调用lua代码的简单示例。

上一节中，我们使用`lua hello.lua`来运行程序。本质在于 Lua 解释器（可执行的 lua）。 Lua 解释器是一个使用 Lua 标准库实现的独立的解释器，她是一 个很小的应用（总共不超过 500 行的代码）。解释器负责程序和使用者的接口：从使用者那里获取文件或者字符串，并传给 Lua 标准库， Lua 标准库负责最终的代码运行。

而我们现在是，`C`作为应用程序语言， `Lua`作为一个库使用，来扩展应用的功能。C 和lua 交互这部分称为 C API

**C API 是一个 C 代码与 Lua 进行交互的函数集。他有以下部分组成：读写 Lua 全局变量的函数，调用 Lua 函数的函数**，运行 Lua 代码片断的函数，注册 C 函数然后可以在 Lua 中被调用的函数，等等。

```cpp
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <iostream>

int main()
{
    lua_State *L = luaL_newstate();

    luaL_openlibs(L);

    lua_getglobal(L, "print");

    std::string str = "hello world";

    lua_pushstring(L, str.c_str());

    lua_call(L, 1, 0);

    return 0;
}
```

编译的时候，需要指定lua头文件位置，链接下lua库。

```shell
g++ 2_hello.cpp -o 2_hello  -I /usr/include/lua5.3  -llua5.3
```

运行程序，输出如下。

```shell
./2_hello
hello world
```

下面简单介绍下，上面代码。

* 头文件 lua.h 定义了 Lua 提供的基础函数。所有在 lua.h 中被定义的都有一个 lua_前缀。头文件 lauxlib.h 定义了辅助库（auxlib）提供的函数。同样，所有在其中定义的函数等都以 luaL_打头。辅助库利用 lua.h 中提供的基础函数提供了更高层次上的抽象。

* Lua 库没有定义任何全局变量。它所有的状态保存在动态结构 lua_State 中，而且指向这个结构的指针作为所有 Lua 函数的一个参数。这样的实现方式使得 Lua 能够重入且为在多线程中的使用作好准备。

* [luaL_newstate](https://www.bookstack.cn/read/lua-5.3/spilt.40.spilt.1.5.md)创建一个新的 Lua 状态机。如果内存分配失败，则返回 `NULL` 。这个状态机中没有包含任何函数。

* 代码使用[luaL_openlibs](https://www.bookstack.cn/read/lua-5.3/spilt.41.spilt.1.5.md)打开指定状态机中的所有 Lua 标准库。

* Lua用一个抽象的栈在 Lua 与 C 之间交换值。栈中的每一条记录都可以保存任何 Lua 值。**无论你何时想要从 Lua 请求一个值（比如一个全局变量的值），调用 Lua，被请求的值将会被压入栈。无论你何时想要传递一个值给 Lua，首先将这个值压入栈，然后调用 Lua（这个值将被弹出）**。

* 使用`lua_getglobal`获取`print`函数地址，地址保存在栈中。将`print`函数需要的参数压栈。[lua_call](https://www.bookstack.cn/read/lua-5.3/spilt.8.4.md)调用这个函数。(要调用一个函数请遵循以下协议：首先，要调用的函数应该被压入栈；接着，把需要传递给这个函数的参数按正序压栈；这是指第一个参数首先压栈。最后调用一下 `lua_call`)。（如果你了解过汇编，这个很容易理解）（这里画个图会比较好理解，图略。）

---

### C++ 调用lua代码

在介绍一些罗里吧嗦的api之前，我们看一个稍微复杂点的demo。

创建一个lua脚本。在C++中调用这个脚本。

```lua
-- 保存为utils.lua
version = "1.0.0"

function sub(a, b)
  return a - b;
end
```

调用上面脚本的C++程序如下。

```cpp
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <iostream>
#include <string>

int main()
{
    lua_State *L = luaL_newstate();

    luaL_openlibs(L);

    luaL_dofile(L, "./utils.lua");

    lua_getglobal(L, "version");
    std::string version = lua_tostring(L, -1);
    std::cout << version << std::endl;
    
    lua_getglobal(L, "sub");
    lua_pushnumber(L, 10);
    lua_pushnumber(L, 2);
    lua_call(L, 2, 1);
    int sub_result = lua_tonumber(L, -1);
    std::cout << sub_result << std::endl;

    return 0;
}
```

编译和运行过程，和之前相同。

```shell
g++ utils_test.cpp -o utils_test  -I /usr/include/lua5.3  -llua5.3
./utils_test                                                      
1.0.0
8
```

### 一些罗里吧嗦的API

略。
