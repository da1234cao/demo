#pragma once

#include "util.hpp"
#include <unistd.h>
#include <string.h>
#include <cassert>
#include <string>

#define TMP_BUFFER_SIZE 2048

enum resolve_status {
  READABLE,
  WRITEBALE,
  UNKNOW
};

class resolve {
private:
  char tmp_buf[TMP_BUFFER_SIZE];
private:
  resolve_status m_status = UNKNOW;
  std::string m_recv_buf;
  std::string m_send_buf;
  int m_epollfd;
  int m_sockfd;
public:
  resolve(int epollfd, int sockfd): m_epollfd(epollfd), m_sockfd(sockfd) {}
  void process() {
    if(m_status == UNKNOW || m_status == READABLE) {
      // 读处理
      int bytes_read = read(m_sockfd, tmp_buf, TMP_BUFFER_SIZE);
      assert(bytes_read >= 0);
      m_recv_buf.append(std::string(tmp_buf, bytes_read));

      // demo假定这里一次读取完毕，进行状态转换
      // 同时设置相关联的fd可写
      m_status = WRITEBALE;
      if(m_status == WRITEBALE) {
        utils::epoll_help::instance().modfd(m_epollfd, m_sockfd, EPOLLOUT);
      }
    }else if(m_status == WRITEBALE) {
      // 写处理
      m_send_buf = m_recv_buf;
      m_recv_buf.clear();
      int sended = 0;
      while(sended < m_send_buf.size()) {
        int n = write(m_sockfd, m_send_buf.c_str() + sended, m_send_buf.size() - sended);
        sended += n;
      }

      // 写完毕，进行状态转换
      m_status = READABLE;
      if(m_status == READABLE) {
        utils::epoll_help::instance().modfd(m_epollfd, m_sockfd, EPOLLIN);
      }
    }
  }
};