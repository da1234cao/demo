# cmake构建动态库

## 前言

linux下的动态库/静态库，以前就知道咋构建：[头文件与库的关系](https://da1234cao.blog.csdn.net/article/details/108326511)。最近写win下动态库的时候，有些细节与linux下有点不同。

所以这篇文章简单介绍下，如果使用cmake构建一个可以跨平台编译使用的动态库。（我尝试使用[cmake-option](https://cmake.org/cmake/help/latest/command/option.html), 通过一个CMakeLists.txt，分别编译生成动态库/静态库。完全可以做到，但是要让头文件区分动态库/静态库有点麻烦。所以本文缩小范围，仅仅考虑动态库的跨平台构建）

详细代码见仓库

## 代码

代码结构

```shell

```

### 库文件代码

下面是头文件和头文件中接口的实现。包含全局变量，函数，类在动态库中的写法。

头文件

```cpp
#pragma once

#include <iostream>
#include <string>

#ifdef _WIN32
    #ifdef _EXPORT_DLL_
        #define EXPORT __declspec(dllexport)
    #else
        #define EXPORT __declspec(dllimport)
    #endif
#else
    #define EXPORT
#endif

EXPORT extern const std::string var;

EXPORT void hello();

// EXPORT class hi {
class EXPORT hi {
public:
    hi() {
        std::cout << "hello construct." << std::endl;
    }
};
```

接口实现。

```cpp
#include "hello.h"

const std::string var = "hello var";

void hello() {
    std::cout << "hello world" << std::endl;
}
```

下面按点罗列其中的注意点。

* 众所周知，全局变量写在cpp中，在头文件中使用extern, 避免多次引用导致的重定义问题。
* 区分不同平台的宏定义，可以参考：[How to detect reliably Mac OS X, iOS, Linux, Windows in C preprocessor? [duplicate]](https://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor)。`_WIN32`表win平台，包含win32和win64。
* 关于dllexport和dllimport，可以看些这几个链接：[dllimport与dllexport作用与区别 - de0319gh - 博客园](https://www.cnblogs.com/de0319gh/p/3254013.html)、[visual c++ - What is the difference between dllexport and dllimport? - Stack Overflow](https://stackoverflow.com/questions/57999/what-is-the-difference-between-dllexport-and-dllimport)、[使用 __declspec(dllexport) 从 DLL 导出 | Microsoft Learn](https://learn.microsoft.com/zh-cn/cpp/build/exporting-from-a-dll-using-declspec-dllexport?view=msvc-170)、[使用 DEF 文件从 DLL 导出 | Microsoft Learn](https://learn.microsoft.com/zh-cn/cpp/build/exporting-from-a-dll-using-def-files?view=msvc-170)。
  * 使用 **`__declspec(dllexport)`** 关键字从 DLL 中导出数据、函数、类或类成员函数(到lib文件中)。若要导出函数，**`__declspec(dllexport)`** 关键字必须出现在调用约定关键字的左侧；若要导出类中的所有公共数据成员和成员函数，该关键字必须出现在类名的左侧；
  * 使用`__declspec(dllimport)`会更好(为什么会更好，可以不理会)。如果使用动态库中的全局变量，则必须使用`__declspec(dllimport)`。

### 测试代码

测试代码如下。

```cpp
#include "../lib/hello.h"

int main(int argc, char* argv[]) {
    hi h;
    hello();
    std::cout << var << std::endl;
    return 0;
}
```

```cmake
cmake_minimum_required(VERSION 3.10.0)
 
project(test)

link_directories(${PROJECT_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} test.cpp)
 
target_link_libraries(${PROJECT_NAME} hello)  
```

编译运行结果如下。

```shell
mkdir build
cd build
cmake ..
cmake --build . --config Release
..\bin\Release\test.exe
hello construct.
hello world
hello var
```

查看下程序依赖的动态库。

```shell
 dumpbin.exe /dependents  bin\Release\test.exe
  Image has the following dependencies:

    hello.dll  <- 这里
    MSVCP140.dll
    ....
    KERNEL32.dll
```

查看下动态库的导出信息。

```shell
 dumpbin.exe /EXPORTS .\bin\Release\hello.lib
 File Type: LIBRARY

     Exports

       ordinal    name

                  ??0hi@@QEAA@XZ (public: __cdecl hi::hi(void))
                  ??4hi@@QEAAAEAV0@$$QEAV0@@Z (public: class hi & __cdecl hi::operator=(class hi &&))
                  ??4hi@@QEAAAEAV0@AEBV0@@Z (public: class hi & __cdecl hi::operator=(class hi const &))
                  ?hello@@YAXXZ (void __cdecl hello(void))
                  ?var@@3V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@B (class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > const var)
```

下面按点罗列其中的注意点。

* win下`cmake --build .`默认生成的是debug版本，参见：[windows - How to change the build type to Release mode in cmake? - Stack Overflow](https://stackoverflow.com/questions/19024259/how-to-change-the-build-type-to-release-mode-in-cmake)

* 如果动态库和静态库在相同目录，想优先链接动态库，不太容易做到，见：[CMake优先链接静态库](https://www.cnblogs.com/coding-my-life/p/14018972.html)、[How do I tell CMake to link in a static library in the source directory? - Stack Overflow](https://stackoverflow.com/questions/14077611/how-do-i-tell-cmake-to-link-in-a-static-library-in-the-source-directory)

* dumpbin.exe命令输出的符号信息，应该是修饰名。我没细看规则，可参考：[[转][C/C++]函数名字修饰（Decorated&#160;Name）方式 - 每天进步一点点点 - 博客园](https://www.cnblogs.com/victor-ma/p/4184806.html)、[修饰名 | Microsoft Learn](https://learn.microsoft.com/zh-cn/cpp/build/reference/decorated-names?view=msvc-170)

### Linux下测试

```shell
# 编译
cmake -DCMAKE_BUILD_TYPE=release --build .
make

# 运行
../bin/test
hello construct.
hello world
hello var

# 查看程序依赖库
ldd ../bin/test
        linux-vdso.so.1 (0x00007ffdc1ffd000)
        libhello.so => xxx/bin/libhello.so (0x00007f761396d000) 《--这里
        libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f7613774000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f7613582000)
        libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f7613433000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f7613979000)
        libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f7613418000)




```
