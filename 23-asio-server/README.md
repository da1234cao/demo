[toc]

# 使用asio实现一个单线程异步的socket服务程序

## 前言

之前，我使用`epoll`实现过一个C++的后端服务程序，见：[从头开始实现一个留言板-README_c++做一个留言板_大1234草的博客-CSDN博客](https://da1234cao.blog.csdn.net/article/details/127876008)

但是它不够简便，无法轻松的合并到其他代码中。并且，由于程序中使用`epoll`函数，代码无法运行在windows上。如果选用`select`，倒是可以同时在windows和linux上运行。

所以，**本文重写一个C++后端服务程序，它需要便捷的嵌入到不同的项目中，需要可以跨平台运行**。

网上可能有很多这样的程序，比如：[【C++】HTTP Server 开源库（汇总级别整理）&#39; | 像我这样的人](https://ericpengshuai.github.io/c-c/039eca2b3cfe.html)。但是，处于菜鸟阶段，我得多敲代码（我暂时也没有去对比不同的实现，因为拖着拖着，就会不想做了）。

这里，我们看下有哪些技术路线。第一种是套接字编程。 `socket`在windows和linux上的使用，略有区别。所以，这里不选择直接进行套接字编程。第二种是C++的网络库。C++的标准库没有提供网络功能，所以这里选择使用[GitHub - boostorg/asio: Boost.org asio module](https://github.com/boostorg/asio)。

初次使用`boost::asio`，会有点难上手。但是，本文不介绍`boost::asio`的具体使用，因为太冗长。这里推荐下参考资料。

* 《Boost程序完全开发指南》12.3 asio  -- 入门级介绍，可以有个初步认知

* [GitHub - dongzj1997/Web-Server: A simple and fast HTTP server implemented using C++17 and Boost.Asio.](https://github.com/dongzj1997/Web-Server/tree/master) -- 实战教程，可以上手写asio网络代码

* [C++11 Examples - 1.81.0](https://www.boost.org/doc/libs/1_81_0/doc/html/boost_asio/examples/cpp11_examples.html#boost_asio.examples.cpp11_examples.http_server)-- 官方教程中的HTTP Server小节，质量不错的代码

本文代码，修改自上面的链接。

详细代码见仓库。

---

## 代码

本文实现的是一个，单线程异步的socket服务程序。

程序的功能是，接收从客户端发送过来的字符串，返回相同的字符串后，关闭连接。

代码的基本结构：

* 封装一个server类，负责功能的启动，停止，信号处理，以及监听(listen)和接收连接(accept)功能。

* 封装一个connect类，围绕已经建立连接的socket，处理读写操作。(因为不同的连接需要不同的存储缓冲区空间，所以每个连接都有个connect对象)

* 封装一个connect_manager类，管理所有的connect对象。

**有很多东西，我还没有完全搞清楚**。

**主要分为五个部分：网络编程中基础知识；C++的基本语法；boost::asio的基本使用；代码的结构设计；不同操作系统的API的使用，以实现相同的功能**。

* socket中优雅的关闭。

* 端口复用与地址复用：[socket 端口复用 SO_REUSEPORT 与 SO_REUSEADDR - schips - 博客园](https://www.cnblogs.com/schips/p/12553321.html)

* C++中的左值，右值，移动语义，错误码等。

* asio中单线程异步的基本原理，asio的多线程编程。

* 代码的结构设计。(本文的代码参考自之前的链接。代码结构中，很好的一点是使用connect_manager去管理connect。当connect释放的时候，connect调用的是connect_managet中的方法。从而，避免了在server中做这件事。这样的结构很好。)

* Linux中信号，windows中信号，windows中事件，这三者的区别。

下面是具体代码。

首先是main函数代码。

```cpp
#include "server.h"

int main(int argc, char *argv[])
{
    server s("127.0.0.1","6666");
    s.run();
    return 0;
}
```

下面是server类的封装。

```cpp
#pragma once
#include "connection.h"
#include <boost/asio.hpp>

class server {
public:
  server(const std::string& address, const std::string& port);
  void run();
  void stop();
private:
  void do_accept();
private:
  boost::asio::io_context m_io_context;
  boost::asio::ip::tcp::acceptor m_acceptor;
  connection_manager m_connection_manager;
  boost::asio::signal_set m_signals;
};
```

```cpp
#include "server.h"
#include <iostream>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind/bind.hpp>
#include <fstream>

server::server(const std::string& address, const std::string& port)
  : m_io_context(1), 
    m_acceptor(m_io_context),
    m_connection_manager(),
    m_signals(m_io_context)
{
  // 在win下，使用taskkill发送信号，会让进程直接退出，并没有执行这里的信号处理。
  // 目前不清楚，可参考：https://stackoverflow.com/questions/26404907/gracefully-terminate-a-boost-asio-based-windows-console-application
  m_signals.add(SIGINT);
  m_signals.add(SIGTERM);
  m_signals.async_wait(
      [this](boost::system::error_code ec, int signo)
      {
        if(signo == SIGINT || signo == SIGTERM) {
          stop();
        }
      });

  boost::asio::ip::tcp::resolver resolver(m_io_context);
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
  m_acceptor.open(endpoint.protocol());
  m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  m_acceptor.bind(endpoint);
  m_acceptor.listen();

  do_accept();
}

void server::do_accept()
{
  // Move accept handler requirements
  m_acceptor.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
      {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!m_acceptor.is_open()) {
          return;
        }
        if (!ec) {
          m_connection_manager.start(std::make_shared<connection>(std::move(socket), m_connection_manager));
        }
        
        do_accept();
      });
}

void server::run()
{
  m_io_context.run();
}

void server::stop()
{
  // 服务器停止是通过取消所有未完成的异步操作来实现的。
  // 一旦所有操作都完成，io_context::run() 函数将退出。
  m_acceptor.close();
  m_connection_manager.stop_all();
}
```

接下来是connect类。我把connect_manager类，也放在同一个文件里面了。

```cpp
#pragma once
#include <memory>
#include <set>
#include <boost/asio/ip/tcp.hpp>

class connection;
typedef std::shared_ptr<connection> connection_ptr;

class connection_manager {
public:
  connection_manager() = default;
  connection_manager(const connection_manager&) = delete;
  connection_manager& operator=(const connection_manager&) = delete;

  void start(connection_ptr c);
  void stop(connection_ptr c);
  void stop_all();

private:
  std::set<connection_ptr> m_connections;
};



class connection : public std::enable_shared_from_this<connection> {
public:
  connection(const connection&) = delete;
  connection& operator=(const connection&) = delete;

  connection(boost::asio::ip::tcp::socket socket, connection_manager& manager);

  void start();

  void stop();

private:
  void do_read();
  void do_write();
  void handle_read(const boost::system::error_code& ec, size_t bytes_transferred);
  void handle_write(const boost::system::error_code& ec, size_t bytes_transferred);

private:
  boost::asio::ip::tcp::socket m_socket;
  int m_write_size = 0;
  std::array<char, 4096> m_read_buffer;
  std::array<char, 4096> m_write_buffer;
  connection_manager& m_connection_manager;

};
```

```cpp
#include "connection.h"
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>

void connection_manager::start(connection_ptr c)
{
    m_connections.insert(c);
    c->start();
}

void connection_manager::stop(connection_ptr c)
{
  m_connections.erase(c);
  c->stop();
}

void connection_manager::stop_all()
{
  for (auto c: m_connections)
    c->stop();
  m_connections.clear();
}

connection::connection(boost::asio::ip::tcp::socket socket,
    connection_manager& manager)
  : m_socket(std::move(socket)),
    m_connection_manager(manager)
{
}


void connection::start()
{
    do_read();
}

void connection::do_read()
{
  m_socket.async_read_some(boost::asio::buffer(m_read_buffer),
              boost::bind(&connection::handle_read, shared_from_this(), 
                          boost::asio::placeholders::error, 
                          boost::asio::placeholders::bytes_transferred));
}

void connection::handle_read(const boost::system::error_code& ec, size_t bytes_transferred)
{
  if(!ec) {
    // 检查是否接受到完整的信息；(这里假定收到的信息完整)
    // 单线程的异步程序，这里会存在问题麻？
    m_write_buffer.fill('\0');
    m_write_buffer = m_read_buffer;
    m_write_size = bytes_transferred;
    do_write();
  }
  else if(ec != boost::asio::error::operation_aborted){
    m_connection_manager.stop(shared_from_this());
  }
}

void connection::do_write()
{
  m_socket.async_write_some(boost::asio::buffer(m_write_buffer.data(), m_write_size),
              boost::bind(&connection::handle_write, shared_from_this(), 
                          boost::asio::placeholders::error, 
                          boost::asio::placeholders::bytes_transferred));
}

void connection::handle_write(const boost::system::error_code& ec, size_t bytes_transferred)
{
  if(!ec) {
    // 发送后断开连接
    m_connection_manager.stop(shared_from_this());   
    // 这里的写法比较神奇.调用管理者来释放自己.
    // 直接调用connection::stop，会导致connection_manager中该对象的智能指针没有删除(虽然在对象释放后这个智能指针可能指向为空) 
  }
  else if(ec != boost::asio::error::operation_aborted) {
    m_connection_manager.stop(shared_from_this());
  }
}

void connection::stop()
{
  // m_socket.close();
  // 优雅的关闭：发送缓冲区中的内容发送完毕后再完全关闭
  boost::system::error_code ignored_ec;
  m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
}
```
