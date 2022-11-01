#pragma once

#include "http_parser.h"
#include "string.h"
#include <iostream>
#include <string>
#include <functional>

namespace http{


class request_base {
private:
  char *m_data = nullptr;
  int m_len = 0;
  http_parser m_parse;
  http_parser_settings m_settings;

private:
  std::string m_latest_field;

public:
  typedef std::function<int (http_parser *p, const char *buf, size_t len)> field_function_type;
  typedef  int(field_function_ptr_type)(http_parser*, const char*, size_t) ;
  request_base() {
    http_parser_init(&m_parse, HTTP_REQUEST);
    m_settings = {
      .on_message_begin = 0
      ,.on_url = 0
      ,.on_status = 0
      ,.on_header_field = 0
      ,.on_header_value = 0
      ,.on_headers_complete = 0
      ,.on_body = 0
      ,.on_message_complete = 0
      ,.on_chunk_header = 0
      ,.on_chunk_complete = 0
    };
  }

  request_base(char* data, int len): request_base() {
    m_data = data; 
    m_len = len;
  }

  void data(char* data, int len) {
    m_data = data;
    m_len = len;
  }

  void set_latest_field(std::string &latest_field) {
    m_latest_field = latest_field;
  }

  template<class T>
  void set_on_header_field_cb(T header_field_cb) {
    m_settings.on_header_field = header_field_cb.target();
  }


};

int on_header_field_cb(request_base *req, http_parser *p, const char *buf, size_t len) {
    std::string latest_field = std::string(buf, len);
    req->set_latest_field(latest_field);
    return 0;
}

class request : request_base {
  request() = default;
  request(char* data, int len): request_base(data, len) {
    // here compile err. how can  I store the bind return? （without use auto）
    // https://stackoverflow.com/questions/13238050/convert-stdbind-to-function-pointer
    // https://stackoverflow.com/questions/10692121/c-bind-function-for-use-as-argument-of-other-function/74259581#74259581
    auto header_field_cb = std::bind(on_header_field_cb, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    this->set_on_header_field_cb(header_field_cb);
  }
};




} // http namespace