#pragma once
#include "connection.h"
#include <boost/asio.hpp>

class server {
public:
  server(const std::string& address, const std::string& port);
  void run();
private:
  void do_accept();
private:
  boost::asio::io_context m_io_context;
  boost::asio::ip::tcp::acceptor m_acceptor;
  connection_manager m_connection_manager;
  boost::asio::signal_set m_signals;
};