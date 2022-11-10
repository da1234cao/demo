#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <boost/json.hpp>


using namespace std;

int main(int argc, char** argv)
{
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
  cout << info_json_str << endl;
}