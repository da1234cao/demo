#include <boost/beast/http.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/json.hpp>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

std::string json_body() {
  vector<pair<string, string>> info;
  info.push_back(make_pair("张三","你好世界"));
  info.push_back(make_pair("Bob", "hello world"));

  boost::json::array info_array;
  for(auto &in : info) {
    boost::json::object obj;
    obj["name"] = in.first;
    obj["message"] = in.second;
    info_array.emplace_back(obj);
  }
  boost::json::object info_json;
  info_json.emplace("info", info_array);

  string info_json_str = boost::json::serialize(info_json);
  return info_json_str;
}

int main(int argc, char** argv)
{
  namespace http = boost::beast::http;
  http::response<http::string_body> resp;
  resp.set(http::field::server, "tiny-server"); // 处理请求的软件
  resp.set(http::field::access_control_allow_origin, "*"); // 允许跨与访问
  resp.set(http::field::content_type, "application/json;charset=utf8"); // 返回的内容类型
  resp.body() = json_body(); // body内容
  resp.prepare_payload(); // 根据body的长度，调整Content-Length
  resp.result(http::status::ok);  // 响应值为200

  // 将response转换成字符串
  // https://stackoverflow.com/questions/71514303/how-to-convert-httpresponsehttpstring-bodybase-to-stdstring
  // https://github.com/boostorg/beast/issues/819
  const std::string str_headers = boost::lexical_cast<std::string>(resp.base()); 
  std::string str_body = resp.body().data();
  std::cout << str_headers << str_body;
}