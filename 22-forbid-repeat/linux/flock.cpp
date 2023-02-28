#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <boost/scope_exit.hpp>
#include <iostream>


int main(int argc, char *argv[])
{
  int fd = open("./flock.lock", O_WRONLY|O_CREAT);
  if(fd < 0) {
    std::cout << "打开锁文件失败." << std::endl;
    return -1;
  }

  if(flock(fd, LOCK_EX | LOCK_NB) == -1) {
    std::cout << "程序已经运行，禁止重新启动" << std::endl;
    return -1;
  }
  BOOST_SCOPE_EXIT(&fd) {
    flock(fd, LOCK_UN); // 不执行这步，会导致open失败，暂时还每搞清楚文件锁的使用
  }BOOST_SCOPE_EXIT_END

  std::cout << "程序睡眠中..." << std::endl;
  sleep(5);
  
  return 0;
}