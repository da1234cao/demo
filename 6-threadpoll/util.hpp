#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>

namespace utils {
class semaphore {
private:
  int m_count = 0; // 共享变量
  std::mutex m_mutex;
  std::condition_variable m_cv;

public:
  semaphore() = default;
  semaphore(int count) : m_count(count) {}
  semaphore(const semaphore&) = delete;
  semaphore& operator=(const semaphore&) = delete;

  void post() {
    // 加锁进行互斥操作：修改共享变量，并通知阻塞线程
    std::unique_lock<std::mutex> lock(m_mutex);
    m_count++;
    m_cv.notify_one();
  }

  void wait() {
    // 加锁进行互斥操作：使用条件变量阻塞当前线程，直到（使用共享变量的）谓词条件满足
    std::unique_lock<std::mutex> lock(m_mutex); 
    m_cv.wait(lock, [=] {return m_count > 0;});
    m_count--;
  }
};

// come from: https://blog.csdn.net/liuxuejiang158blog/article/details/17301739
template<typename T>
class threadsafe_queue
{
  private:
     mutable std::mutex mut; 
     std::queue<T> data_queue;
     std::condition_variable data_cond;
  public:
     threadsafe_queue(){}
     threadsafe_queue(threadsafe_queue const& other)
     {
         std::lock_guard<std::mutex> lk(other.mut);
         data_queue=other.data_queue;
     }
     void push(T new_value)//入队操作
     {
         std::lock_guard<std::mutex> lk(mut);
         data_queue.push(new_value);
         data_cond.notify_one();
     }
     void wait_and_pop(T& value)//直到有元素可以删除为止
     {
         std::unique_lock<std::mutex> lk(mut);
         data_cond.wait(lk,[this]{return !data_queue.empty();});
         value=data_queue.front();
         data_queue.pop();
     }
     std::shared_ptr<T> wait_and_pop()
     {
         std::unique_lock<std::mutex> lk(mut);
         data_cond.wait(lk,[this]{return !data_queue.empty();});
         std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
         data_queue.pop();
         return res;
     }
     bool try_pop(T& value)//不管有没有队首元素直接返回
     {
         std::lock_guard<std::mutex> lk(mut);
         if(data_queue.empty())
             return false;
         value=data_queue.front();
         data_queue.pop();
         return true;
     }
     std::shared_ptr<T> try_pop()
     {
         std::lock_guard<std::mutex> lk(mut);
         if(data_queue.empty())
             return std::shared_ptr<T>();
         std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
         data_queue.pop();
         return res;
     }
     bool empty() const
     {
         std::lock_guard<std::mutex> lk(mut);
         return data_queue.empty();
     }
};
} // utils namespace