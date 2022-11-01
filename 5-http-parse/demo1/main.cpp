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

