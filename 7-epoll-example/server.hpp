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