#include "signalproc.h"

signalproc::signalproc(std::function<void(int)> single_hanlde) {
  // 后面必须调用set_hanlde
  act.sa_handler = single_hanlde.target();

  // 当进入信号处理函数的时候，阻塞所有信号
  // 仅当从信号处理函数返回时，再将信号的屏蔽字设置为恢复为原先的值
  sigfillset(&act.sa_mask);

  // 有此信号中断的系统调用自动重启
  act.flag = SA_RESTART;
}

signalproc::add(int signo) {
  sigaction(signo, act, NULL);
}