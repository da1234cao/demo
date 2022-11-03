[toc]

## 前言

本文完整代码见仓库。

本文实现的是一个，可以在server中使用的线程池，类似于[qinguoyi/TinyWebServer](https://github.com/qinguoyi/TinyWebServer/blob/master/threadpool/threadpool.h)。

关键字：信号量、线程安全队列、线程池

---

## 使用互斥锁和条件变量实现信号量

linux系统提供了信号量的C接口，可参考[Linux信号量详解](http://c.biancheng.net/view/8632.html)。

在C++标准库中，[C++20提供semaphore](https://zh.cppreference.com/w/cpp/header/semaphore)。这个版本有点高，所以得自行封装一个。

c++使用linux的信号接口，封装出一个信号量也不难，但是这样的代码只能在Linux上运行，没有跨平台性。那不如直接调用C++的互斥锁和条件变量来实现信号量，比如：[C++ 并发编程（六）：信号量（Semaphore）](https://segmentfault.com/a/1190000006818772)、[信号量 Semaphore 及 C++ 11 实现-知乎](https://zhuanlan.zhihu.com/p/512969481)

上面两个在功能封装上没啥问题，但是封住成类的时候，我们应该还得考虑类自身特性，比如能否拷贝或者继承。比较明显的时候，信号量不应该拷贝(复制构造和复制赋值)，所以我添加了禁止拷贝的成员函数。

另外，我在描述下`unique_lock`和`condition_variable`，因为我之前没用过，所以查了下。

我知道[lock_guard](https://zh.cppreference.com/w/cpp/thread/lock_guard),满足RAII，构造时候加锁，析构的时候解锁。而[unique_lock](https://zh.cppreference.com/w/cpp/thread/unique_lock)比lock_guard更灵活些。主要是unique_lock构造的时候，可以不用mutex进行初始化，使用锁初始化也可以不用占有锁，可参考[C++11 std::unique_lock与std::lock_guard区别及多线程应用实例](https://www.cnblogs.com/fnlingnzb-learner/p/9542183.html)。（或许我以后大多数情况下都使用unique_lock，而不使用lock_guard）。

[condition_variable](https://zh.cppreference.com/w/cpp/thread/condition_variable)类是同步原语，能用于阻塞一个线程，或同时阻塞多个线程，直至另一线程修改共享变量（条件）并通知condition_variable。(这句话很简洁，但也很实在。)展开来看，可以参考[条件变量condition_variable的使用及陷阱](https://www.cnblogs.com/fenghualong/p/13855360.html)。**总的来说，使用条件变量需要三个元素：锁(mutex)，共享变量，条件变量。而使用条件变量则需要这个三个元素搭配工作，来实现阻塞当前线程和通知阻塞线程**。(首先是阻塞当前线程：谓词需要使用共享的变量，决定在什么条件下唤醒线程；而在通知阻塞线程前，则需要修改共享变量，保证通知线程的时候，有线程满足谓词条件，可以被唤醒工作。)

```cpp
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
} // utils namespace
```

这里扣一个小细节。设想这样一种情况：信号量的初始值为零；线程a调用的信号量的wait函数；接着线程b调用了post函数。这会不会在造成线程a一直占用着锁，线程b即无法调用post，也无法调用wait呢？

答案是不会。因为[std::condition_variable::wait](https://zh.cppreference.com/w/cpp/thread/condition_variable/wait)中明确写道：wait 导致当前线程阻塞直至条件变量被通知，**原子地解锁 lock，阻塞当前执行线程，并将它添加到于 *this上等待的线程列表**。线程将在执行 notify_all()或notify_one()时被解除阻塞。解阻塞时，无关乎原因， lock再次锁定且wait退出。

---

## 寻找一个线程安全的队列

[std::queue](https://zh.cppreference.com/w/cpp/container/queue)并不是一个线程安全的数据结构。

```cpp
template<
    class T,
    class Container = std::deque<T>
> class queue;
```

想要一个线程安全的队列的第一个想法是，将std::queue进行加锁封装。安全队列封装的评价标准或许有这几点：
* 第一点：在封装之后，功能是否有缺失， 比如[C++并发实战12：线程安全的queue](https://blog.csdn.net/liuxuejiang158blog/article/details/17301739),这个封装之后的安全队列，只能使用deque作为容器。因为封装的时候，模板上比原来少了一个参数。但是总体来看，封装的还不错。
* 第二点：封装之后，带来安全，却损失了效率。

第二个想法是，boost提供了一个线程安全的队列：[boost-lockfree-queue](https://www.boost.org/doc/libs/1_80_0/doc/html/boost/lockfree/queue.html)。有个问题是，**什么是lockfree**,我看了下文档，没高明白，可参考[boost::lockfree::queue的原理是什么？是如何管理资源的？-知乎](https://www.zhihu.com/question/415678076)。但是，这里不推荐用。或许有以下几点
* 是一个生产者-消费着模型，没有front/back这样的访问接口，并不是一个纯正的线程安全队列。
* 无法使用`boost::lockfree::queue<std::shared_ptr<T>> m_workqueue;`这样的结构。可参考[[LockFree] a queue of std::shared_ptr?](https://boost-users.boost.narkive.com/o8oduWmn/lockfree-a-queue-of-std-shared-ptr)。boost::lockfree::queue存储的类型有这样的要求：类型存在拷贝构造函数、类型的[析构函数必须是平凡](https://zh.cppreference.com/w/cpp/language/destructor)的、类型的[复制赋值运算符必须是平凡的](https://zh.cppreference.com/w/cpp/language/copy_assignment)。

所以，我们不得不从网上找个封装std::queue的安全队列来使用。(C++真是烦人，啥也没有)

可以用下这个，代码就不粘贴过来了：[C++并发实战12：线程安全的queue](https://blog.csdn.net/liuxuejiang158blog/article/details/17301739)

---

## 线程池-生产者消费者

创建一个线程池，得先搞明白这个线程池的目的是什么。如果需要经常创建线程去执行一个函数，可以参考[基于C++11实现线程池](https://zhuanlan.zhihu.com/p/367309864)。

这里创建的线程池，是个生产者-消费者模型。主线程将任务放入一个工作队列，线程池从阻塞线程中选择一个线程执行。

这个线程池修改自：[高性能服务器编程-threadpool](https://github.com/wgfxcu/HPS/blob/master/15/threadpool/threadpool.h)。原来的代码有些可以改进的地方：其一是，使用了指针，替换使用只能指针；其二是，使用队列加锁进行互斥操作，这里进行了封装；其三是，去除了m_stop成员变量(这个变量毫无意义)。

当然，修改之后的代码，也继承了之前代码的缺点，即无法关闭线程池。**生产者-消费者模型的线程池，当工作队列为空的时候,消费者线程阻塞。此时如果主线程退出，由于子线程阻塞，整个进程无法退出**。所以，这个线程池，适用于，放在一个无限循环的结构内，比如作为一个server的线程池。当然，不用担心，当整个进程退出的时候，子线程也会退出，可参考：[When the main thread exits, do other threads also exit?](https://stackoverflow.com/questions/11875956/when-the-main-thread-exits-do-other-threads-also-exit)

下面贴下线程池代码。

```cpp
#pragma once

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
  utils::threadsafe_queue<std::shared_ptr<T>> m_workqueue;

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
  std::shared_ptr<std::thread> thd(new std::thread(&thread_pool::run, this));
  thd->detach();
  m_threads.push_back(thd);
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
    m_workqueue.try_pop(task);
    task->process();
  }
}
```

测试代码如下：

```cpp
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
```
