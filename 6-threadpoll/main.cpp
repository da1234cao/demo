#include "thread_pool.hpp"
#include <iostream>

int main(int argc, char** argv)
{
  typedef struct task {
    int m_index;
    task(int index) : m_index(index) {};
    void process() {
      std::cout << "hello world: " << m_index << std::endl;
    }
  }task;

  std::shared_ptr<task> t1(new task(1));
  std::shared_ptr<task> t2(new task(2));

  thread_pool<task> th_pool(2,5);
  th_pool.append(t1);
  th_pool.append(t2);

  sleep(20); // 应当在无限循环中调用thread_pool,否则主线程退出，由于子线程阻塞，导致阻塞了整个进程的退出
  return 0;
}