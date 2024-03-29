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