#pragma once

#include "http_parser.h"
#include "string.h"
#include <iostream>
#include <string>
#include <functional>
#include <map>

namespace http{
enum request_status {
  // 这里可以添加更多的状态，作状态转移
  REQUEST_INVALID,
  REQUEST_VALID
};

class request {
private:
  char *m_data = nullptr;
  int m_len = 0;
  http_parser m_parse;
  http_parser_settings m_settings;
  std::string m_latest_field;

private:
  std::string m_url;
  std::string m_methed;
  std::map<std::string, std::string> m_headers;
  std::string m_body_buf;
  bool m_request_status = REQUEST_INVALID;

private:
  void set_header(const std::string &value);
  void set_latest_field(std::string);
  void set_url(std::string);
  void set_methed(std::string);
  void append_body(std::string);
  void set_request_status(request_status);

public:
  typedef request self_type;
  request();
  request(const char* data, int len);
  void data(const char* data, int len);
  void execute();
  void get_headers(std::map<std::string, std::string> &headers);
  std::string get_url();
  std::string get_methed();
  std::string get_body();
  bool is_valid();
  static int on_header_field_cb(http_parser *parser, const char *buf, size_t len);
  static int on_url_cb(http_parser *parser, const char *buf, size_t len);
  static int on_header_value_cb(http_parser *parser, const char *buf, size_t len);
  static int on_headers_complete_cb(http_parser *parser);
  static int on_body_cb(http_parser *parser, const char *buf, size_t len);
  static int on_message_complete_cb(http_parser *parser);
};

request::request() {
  http_parser_init(&m_parse, HTTP_REQUEST);
  // 这些函数指针所对应的函数，只能是静态成员方法。要不然类内部，没法获取函数地址。(因为这时候对象没有创建，没法获取地址)
  m_settings = {
    .on_message_begin = 0
    ,.on_url = on_url_cb
    ,.on_status = 0
    ,.on_header_field = on_header_field_cb
    ,.on_header_value = on_header_value_cb
    ,.on_headers_complete = on_headers_complete_cb
    ,.on_body = on_body_cb
    ,.on_message_complete = on_message_complete_cb
    ,.on_chunk_header = 0
    ,.on_chunk_complete = 0
  };

  // 这些回调函数设置为静态方法，所以无法操作类中的成员变量。
  // 幸而，struct http_parser中，留下了一个钩子指针。我们将其赋值为this，这样便可以在static方法中，操作不同的对象的成员变量了
  m_parse.data = this;
}

request::request(const char* data, int len): request() {
  m_data = const_cast<char*>(data); 
  m_len = len;
}

void request::data(const char* data, int len) {
    m_data = const_cast<char*>(data);
    m_len = len;
}

void request::execute() {
  http_parser_execute(&m_parse, &m_settings, m_data, m_len);
}

void request::set_latest_field(std::string latest_field) {
  m_latest_field = latest_field;
}

void request::set_header(const std::string &value) {
  m_headers[m_latest_field] = value;
}

void request::set_url(std::string url) {
  m_url = url;
}

void request::set_methed(std::string methed) {
  m_methed = methed;
}

void request::append_body(std::string body_part) {
  m_body_buf += body_part;
}

void request::set_request_status(request_status status) {
  m_request_status = status;
}

void request::get_headers(std::map<std::string, std::string> &headers) {
  headers = m_headers;
}

std::string request::get_url() {
  return m_url;
}

std::string request::get_methed() {
  return m_methed;
}

std::string request::get_body() {
  return m_body_buf;
}

bool request::is_valid() {
  return m_request_status == REQUEST_VALID;
}

int request::on_header_field_cb(http_parser *parser, const char *buf, size_t len) {
  self_type *impl = reinterpret_cast<self_type*>(parser->data);
  impl->set_latest_field(std::string(buf, len));
  return 0;
}

int request::on_header_value_cb(http_parser *parser, const char *buf, size_t len) {
  self_type *impl = reinterpret_cast<self_type*>(parser->data);
  impl->set_header(std::string(buf, len));
  return 0;
}

int request::on_url_cb(http_parser *parser, const char *buf, size_t len) {
  self_type *impl = reinterpret_cast<self_type*>(parser->data);
  impl->set_url(std::string(buf, len));
  return 0;
}

int request::on_headers_complete_cb(http_parser *parser) {
  self_type *impl = reinterpret_cast<self_type*>(parser->data);
  std::string methed = http_method_str((http_method)parser->method);
  impl->set_methed(methed);
  return 0;
}

int request::on_body_cb(http_parser *parser, const char *buf, size_t len) {
  self_type *impl = reinterpret_cast<self_type*>(parser->data);
  impl->append_body(std::string(buf, len));
  return 0;
}

int request::on_message_complete_cb(http_parser *parser) {
  self_type *impl = reinterpret_cast<self_type*>(parser->data);
  impl->set_request_status(REQUEST_VALID);
  return 0;
}

} // http namespace