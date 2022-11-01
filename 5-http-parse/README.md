## 前言

PS：本文代码见仓库。

背景要求：对http有基本的了解,[http消息简介](https://blog.csdn.net/sinat_38816924/article/details/125275757)

当我们需要从代码层面搭建一个http-server的时候，我们不得不进行http的内容进行解析。

我并不建议自行从源码层面进行解析，比如[http_conn::process_read](https://github.com/qinguoyi/TinyWebServer/blob/9426d2c8abbe3946cc83e74533b1cf61c6f33482/http/http_conn.cpp#L342)-[http状态机](https://mp.weixin.qq.com/s/wAQHU-QZiRt1VACMZZjNlw)

我们需要一些库，来解决这些基础问题。在C++中写代码，是令人不爽快的。我不得不去找http parse的解析库。

第一个是[boost-beast-http](https://www.boost.org/doc/libs/1_80_0/libs/beast/doc/html/beast/using_http.html)。这个需要搭配asio库使用。难搞，因为我现在不想引入C++网络库，所以暂时不用它(也不大会用)。（[Boost.Http - 非官方文档](https://boostgsoc14.github.io/boost.http/#_design_choices)）（C++角度，长远来看，我还是得瞅下这个）

第二个是[nodejs/llhttp - github](https://github.com/nodejs/llhttp)。比较神奇的是，这个仓库通过ts生成C接口的http parse代码。另外可以与其他语言绑定，如python,lua,rust,ruby。我跑了个demo，能用。（还行）

第三个是[nodejs/http-parser - github](https://github.com/nodejs/http-parser)。下面的代码，用的是这个。它不再维护，它是纯C的代码，调用简单。

## request解析

在C++中调用C的接口，还是要封装下的。封装的时候，遇到点麻烦，因为回调函数。

我最开始准备用std::bind+std::function进行封装，但是失败了。失败的代码可以见`demo1/request_fail.hpp`。
* 因为没有搞定bind的返回类型，即该用什么类型去接收/存储一个bind返回值(在不使用auto+模板的前提下)，可见[C++ bind function for use as argument of other function](https://stackoverflow.com/questions/10692121/c-bind-function-for-use-as-argument-of-other-function)
* 另一个没有搞定的是，如何从bind中再提取出一个函数指针，可见[convert std::bind to function pointer](https://stackoverflow.com/questions/13238050/convert-stdbind-to-function-pointer)

封装的时候，遇到另一个麻烦是，只能给回调函数绑定静态的成员函数。（大概是因为非静态的话，此时类还没有创建对应的对象，没法在类内部将类的非静态成员函数的指针地址，赋值给回调函数）但是静态成员函数，是没法直接操作成员变量的。静态成员函数操作其参数倒是完全没问题的。所以在参数中可以访问this指针即可。（所以，**或许可以得出这样的结论：当我们构建一个包含回调的接口的时候，我们需要保留一个void*指针，作为一个hook**）

下面是封装的代码。（没有什么文档吧。主要是看下http-parse的[test.c](https://github.com/nodejs/http-parser/blob/main/test.c#L2665)，看它是如何调用接口的。）(封装的也很烂，不是很全，只是一个基本的接口封装)

```cpp
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
```

下面是测试代码。

```cpp
#include "request.hpp"
#include <iostream>
#include <string>


int main(int argc, char* argv[])
{
  std::string request_str = "GET / HTTP/1.1\r\n"
                        "Host: 180.101.49.13\r\n"
                        "Connection: keep-alive\r\n"
                        "Cache-Control: max-age=0\r\n"
                        "Upgrade-Insecure-Requests: 1\r\n"
                        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\n"
                        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
                        "Accept-Encoding: gzip, deflate\r\n"
                        "Accept-Language: zh-CN,zh;q=0.9\r\n"
                        "Cookie: BD_HOME=1; BD_UPN=12314753; BD_CK_SAM=1; channel=180.101.49.13; baikeVisitId=b4df012b-69e5-400f-9431-21eba73c79a5\r\n"
                        "\r\n";

  http::request req;
  req.data(request_str.c_str(), request_str.length());
  req.execute();

  // print meth
  std::cout << req.get_methed() << std::endl;

  // print url
  std::cout << req.get_url() << std::endl;

  // print headers
  std::map<std::string, std::string> headers;
  req.get_headers(headers);
  for(auto it = headers.begin(); it != headers.end(); it++) {
    std::cout << it->first << ":" << it->second << std::endl;
  }

  // print body
  std::cout << req.get_body() << std::endl;

  // parse is valid
  if(req.is_valid())
    std::cout << "message is valid" << std::endl;
  else
    std:: cout << "message is not valid" << std::endl;
  return 0;
}
```
