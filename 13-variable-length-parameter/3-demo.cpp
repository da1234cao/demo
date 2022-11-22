#include <iostream>
#include <sstream>

using namespace std;

template<typename T>
ostream &print(ostream &os, const T &t) {
  return os << t;
}

template<typename T, typename... Args>
ostream &print(ostream &os, const T &t, const Args&... rest) {
  os << t << " ";
  return print(os, rest...);
}

int main(int argc, char* argv[])
{
  ostringstream oss;
  print(oss, "I wish I had ", 10, " million RMB");
  cout << oss.str() << endl;
}