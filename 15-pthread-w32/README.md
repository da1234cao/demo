[TOC]

## 前言

本文代码见仓库：

最近把[GitHub - giltene/wrk2: A constant throughput, correct latency recording variant of wrk](https://github.com/giltene/wrk2)移植到windows平台上。由于代码遵守[POSIX](https://zh.wikipedia.org/wiki/%E5%8F%AF%E7%A7%BB%E6%A4%8D%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F%E6%8E%A5%E5%8F%A3) ，windows上有相关类似的api，所以移植过程不是很难。

wrk是个压力测试工具。由于我基本没用过压测工具，所以简单的查了下：[压测工具对比 - 金色旭光 - 博客园](https://www.cnblogs.com/goldsunshine/p/16607820.html)、[不同性能压测工具对比_性能测试-阿里云帮助中心](https://help.aliyun.com/document_detail/434264.html)、[常用的HTTP服务压测工具介绍 | 李文周的博客](https://www.liwenzhou.com/posts/Go/benchmark-tools/)

至于wrk工具本身的使用，可以参考：[wrk2入门-http性能压测工具总结_QA的自我修养的博客-CSDN博客_wrk2](https://blog.csdn.net/ccccsy99/article/details/105958366)、[wrk的高级应用总结_QA的自我修养的博客-CSDN博客](https://blog.csdn.net/ccccsy99/article/details/106355347)

本文不谈，如何将linux上的软件移植到windows上来。本文仅仅介绍下[GitHub - GerHobbelt/pthread-win32](https://github.com/GerHobbelt/pthread-win32)的使用。

PS: 我不知道wrk的作者，为啥要用C语言实现。用C++写代码不是快乐的多？

---

## pthread-win32使用

linux操作系统的[pthreads(7) - Linux manual page](https://man7.org/linux/man-pages/man7/pthreads.7.html)，和windows上[创建线程 - Win32 apps | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows/win32/procthread/creating-threads) ，在接口上差别挺大。所以想把Linux上程序移植到windows上，又想尽量少的修改代码，pthread-win32是个很好的选择。（如果是C++，直接用[std::thread - cppreference.com](https://zh.cppreference.com/w/cpp/thread/thread)）

下面我们使用下pthread-win32。网上找了个pthread的demo:[深度探究多线程的效率以及多线程的使用建议_星空_MAX的博客-CSDN博客](https://blog.csdn.net/qq_36653924/article/details/127923102)

编译的时候添加头文件和动态库的路径即可。

```cmake
cmake_minimum_required(VERSION 3.11)

project(thread_test)

include(FetchContent)

# pthread-win32
FetchContent_Declare(
    pthread-win32
    GIT_REPOSITORY https://gitee.com/da1234cao/pthread-win32.git
    # GIT_REPOSITORY https://github.com/GerHobbelt/pthread-win32.git
)
FetchContent_MakeAvailable(pthread-win32)
include_directories(${pthread-win32_SOURCE_DIR})

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} pthreadVC3)
```

```shell
mkdir build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config debug
.\Debug\thread_test.exe
0.352000 seconds
sum:24734094048
```