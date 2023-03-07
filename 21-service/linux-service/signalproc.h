#pragma once
#include <signal.h>
#include <functional>
#include <set>

class signalproc {
public:
  signalproc(std::function<void(int)> single_hanlde);
  void add(int signo);
private:
  struct sigaction act;
};