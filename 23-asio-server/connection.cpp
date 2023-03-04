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