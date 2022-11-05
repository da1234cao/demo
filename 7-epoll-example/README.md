[toc]

## 前言

注：完整代码见仓库。

因为我之前看过[unix网络编程-select函数](https://da1234cao.blog.csdn.net/article/details/125462501)，所以看epoll接口没啥难度。

本文缺点：
* 本文不涉及epoll原理。(因为不懂)
* 本文epoll api介绍较烂。(因为复制粘贴比较麻烦。但阅读下一节的链接，基本能搞明白epoll api的使用)
* 因为我在工作中没有用过epoll，所以没有使用经验。网络编程总是有不实践，不知道的细节。

本文目标：
* 在简单介绍epoll api之后，本文使用epoll实现Reactor模型，实现回射服务器(即，客户端发送的内容，再原样返回)。(Reactor模型介绍，可参考《高性能服务器编程》游双--8.4.1 Reactor模式)（实现的代码和Reactor模型略有区别的是，accept连接过程在主线程中。本文代码，参考提取自：[qinguoyi/TinyWebServer](https://github.com/qinguoyi/TinyWebServer/blob/9426d2c8abbe3946cc83e74533b1cf61c6f33482/webserver.cpp#L377)）


---
## epoll基础

这里主要是介绍下epoll的api。主要是一些复制粘贴的工作。复制粘贴的不全，意思意思，主要看明白书上的概念介绍，在瞅瞅代码进行验证。

来源：
* [epoll(7) — Linux manual page](https://man7.org/linux/man-pages/man7/epoll.7.html)
* 《linux高性能服务器》游双 -- 9.3节epoll系列系统调用 、 **书上对应的代码**：[9-3mtlt.cpp](https://github.com/raichen/LinuxServerCodes/blob/master/9/9-3mtlt.cpp)
* [epoll的简单使用](https://www.cnblogs.com/fnlingnzb-learner/p/5835573.html)、[epoll 多路复用 I/O工作流程](https://wenfh2020.com/2020/04/14/epoll-workflow/)、[【转】Linux下的I/O复用与epoll详解](https://www.itnotebooks.com/?p=1106)

epoll api执行与poll类似的任务: 监视多个文件描述符，以查看它们中的任何一个是否可以执行I/O。epoll api既可以用作边缘触发的接口，也可以用作水平触发的接口，可以很好地扩展到大量可观察的文件描述符。

epoll api的核心概念是epoll实例，它是一种内核数据结构，从用户空间的角度来看，可以将其视为两个列表的容器:
* interest list(或者也被成为epoll set): 监察那些已经被注册的文件描述符
* ready list: 为I/O“就绪”的文件描述符集。就绪列表是epoll set中文件描述符的子集(或者更确切地说，是对文件描述符的一组引用)。由于这些文件描述符上的I/O活动，内核会动态填充就绪列表。

下面是一些api罗列。

`int epoll_create(int size)`:
* epoll把用户关心的文件描述符上的事件放在内核里的一个事件表中， 从而无须像select和poll那样每次调用都要重复传入文件描述符集或事件集。 但epoll需要使用一个额外的文件描述符， 来唯一标识内核中的这个事件表。
* epoll_create创建一个新的 epoll实例。从 Linux2.6.8开始，size 参数被忽略，但是必须大于零。epoll_create返回引用新epoll实例的文件描述符。此文件描述符用于对epoll接口的所有后续调用。当不再需要时，应使用close关闭epoll_create返回的文件描述符。当所有引用epoll实例的文件描述符都已关闭时，内核将销毁该实例并释放相关的资源以供重用。

`int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);`:
* epoll_ctl用来操作epoll的内核事件表
* EPOLL_CTL_ADD， 往事件表中注册fd上的事件; EPOLL_CTL_MOD， 修改fd上的注册事件; EPOLL_CTL_DEL， 删除fd上的注册事件
* fd参数是要操作的文件描述符
* event参数指定事件， 它是epoll_event结构指针类型。epoll_event.events可以设置成这些，EPOLLIN/EPOLLOUT标志epoll_event.data.fd可读/可写。EPOLLET则标志为边缘触发。
  ```cpp
  struct epoll_event
  { 
    __uint32_t events;/*epoll事件*/
    epoll_data_t data;/*用户数据*/
  };

  typedef union epoll_data {
               void        *ptr;
               int          fd;
               uint32_t     u32;
               uint64_t     u64;
  } epoll_data_t;
  ```

`int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);`:
* epoll_wait它在一段超时时间内等待一组文件描述符上的事件
* epoll_wait函数如果检测到事件， 就将所有就绪的事件从内核事件表（由epfd参数指定） 中复制到它的第二个参数events指向的数组中。
* maxevents则是输出输出事件表的最大长度
* 当timeout等于-1的时候这个函数会无限期的阻塞下去，当timeout等于0的时候，就算没有任何事件，也会立刻返回
* 返回值：成功时返回就绪的文件描述符的个数， 失败时返回-1并设置errno

这里不粘贴使用的示例代码。示例代码见上文链接。

---

## epoll使用

写epoll的demo，还是要多线程的。线程池的介绍可参考：[C++线程池](https://blog.csdn.net/sinat_38816924/article/details/127666456)

另外，我们还需要知道下Reactor模式。下面介绍，复制自《高性能服务器编程》游双--8.4.1 Reactor模式。

Reactor是这样一种模式， 它要求主线程（I/O处理单元， 下同） 只负责监听文件描述上是否有事件发生， 有的话就立即将该事件通知工作线程（逻辑单元， 下同） 。 除此之外， 主线程不做任何其他实质性的工作。 读写数据， 接受新的连接， 以及处理客户请求均在工作线程中完成。

1） 主线程往epoll内核事件表中注册socket上的读就绪事件。
2） 主线程调用epoll_wait等待socket上有数据可读。
3） 当socket上有数据可读时， epoll_wait通知主线程。 主线程则将socket可读事件放入请求队列。
4） 睡眠在请求队列上的某个工作线程被唤醒， 它从socket读取数据， 并处理客户请求， 然后往epoll内核事件表中注册该socket上的写就绪事件。
5） 主线程调用epoll_wait等待socket可写。
6） 当socket可写时， epoll_wait通知主线程。 主线程将socket可写事件放入请求队列。
7） 睡眠在请求队列上的某个工作线程被唤醒， 它往socket上写入服务器处理客户请求的结果。

注：代码实现中，accept连接过程，放在了主线程中了。

完整代码见仓库，这里只粘贴下，`server.hpp`代码。

```cpp
#pragma once

#include "resolve.hpp"
#include "thread_pool.hpp"
#include "util.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <cassert>
#include <map>
#include <memory>

#define MAX_EVENT_NUMBER 10000

class server {
private:
  int m_port;
  int m_listenfd;
  int m_epollfd;
  bool m_stop = false;
  thread_pool<resolve> pool; // 处理resolve对象的线程池
  std::map<int, std::shared_ptr<resolve>> resolves; // 存储从客户端发送来的内容，存储回复给客户端的内容

private:
  void event_listen();
  void event_loop();
public:
  server(int port);
  void start();
};

server::server(int port = 9999): m_port(port) {}

void server::start() {
  event_listen(); 
  event_loop();
}

void server::event_listen() {
  // 创建监听描述符并加入epoll set
  m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
  assert(m_listenfd >= 0);

  struct sockaddr_in address;
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(m_port);

  int ret = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));
  assert(ret >= 0);
  ret = listen(m_listenfd, 5);
  assert(ret >= 0);

  m_epollfd = epoll_create(5);
  assert(m_epollfd != -1);
  utils::epoll_help::instance().addfd(m_epollfd, m_listenfd);
}

void server::event_loop()
{
  epoll_event events[MAX_EVENT_NUMBER];
  while(!m_stop) {
    int n = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
    if(n < 0 && errno == EINTR) {
      continue;
    } else if(n < 0 && errno != EINTR) {
      assert(n >=0); // 这里最好抛出异常，先用assert顶顶
    }

    for(int i=0; i<n; i++) {
      int sockfd = events[i].data.fd;
      if(sockfd == m_listenfd) { // 建立新的连接
        struct sockaddr_in client_address;
        socklen_t client_addr_len = sizeof(client_address);
        int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addr_len);
        assert(connfd >= 0); // 这里应当使用异常处理

        utils::epoll_help::instance().addfd(m_epollfd, connfd);
        resolves[connfd] = std::shared_ptr<resolve>(new resolve(m_epollfd, connfd));
      } else if(events[i].events & EPOLLIN) { // 读-丢给线程池处理
        pool.append(resolves[sockfd]);
      } else if(events[i].events & EPOLLOUT) { // 写-丢给线程池处理
        pool.append(resolves[sockfd]);
      } else if(events[i].events & EPOLLRDHUP) { // 客户端关闭
        utils::epoll_help::instance().removefd(m_epollfd, events[i].data.fd);
        // resolves.erase(events[i].data.fd); // 这样擦出或许不好，要改变树结构
        // 所以不用删除，fd已经关闭，不会使用resolves[fd];当新的相同的fd连接时，自动覆盖
      }
    }
  }
}
```

测试的客户端代码没写。使用`nc 127.0.0.1 9999`进行连接测试即可。