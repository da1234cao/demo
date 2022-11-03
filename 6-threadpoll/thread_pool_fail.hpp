#pragma once

// [LockFree] a queue of std::shared_ptr?: https://boost-users.boost.narkive.com/o8oduWmn/lockfree-a-queue-of-std-shared-ptr

#include "util.hpp"
#include <boost/lockfree/queue.hpp>
#include <vector>
#include <thread>

template<typename T>
class thread_pool {
private:
  utils::semaphore sem;
  int m_max_thread_num;
  int m_max_queue_size;
  std::vector<std::shared_ptr<std::thread>> m_threads;
  boost::lockfree::queue<std::shared_ptr<T>> m_workqueue;


public:
  thread_pool(const thread_pool&) = delete;
  thread_pool& operator=(const thread_pool&) = delete;
  thread_pool(int max_thread_num = 3, int m_max_queue_size = 100);
  void append(std::shared_ptr<T> task);
  void run();
  // ~thread_pool();
};

template<typename T>
thread_pool<T>::thread_pool(int max_thread_num, int m_max_queue_size) {
  std::shared_ptr<T> task(new std::thread(&thread_pool::run, this));
  task->detach();
  m_workqueue.push(task);
}

template<typename T>
void thread_pool<T>::append(std::shared_ptr<T> task) {
  // 工作队列中放入一个任务；使用信号量唤醒一个线程
  m_workqueue.push(task);
  sem.post();
}

template<typename T>
void thread_pool<T>::run() {
  // 运行一边结束后，为了让线程不退出，加了一个循环
  // 当append之后，随机一个线程run
  while (1) {
    sem.wait();
    std::shared_ptr<T> task;
    m_workqueue.pop(task);
    task->process();
  }
}