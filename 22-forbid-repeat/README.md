[toc]

## 前言

大多数时候，我们不允许一个程序有多个实例。即，当一个程序正在运行的时候，禁止该程序再次运行。

有很多方法实现。这里简单罗列下我碰到的。

(我想睡觉了，这里简单列下。那天心情好，再补上描述)

---

## win系统

### 互斥对喜

```cpp
#include <windows.h>

int main(int argc, char **argv)
{
  // 程序结束后，自动释放该锁;也可以closehandle(hmutex)下
  HANDLE hmutex = CreateMutex(NULL, FALSE, "FORBID_REPEAT_MUTEXT");
  if(hmutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS ) {
    MessageBox(NULL, TEXT("程序已经启动,禁止重复启动"), TEXT("提示"), MB_OK);
    return -1;
  }
  return 0;
}
```

---

### linux系统

未细读：[被遗忘的桃源——flock 文件锁](https://zhuanlan.zhihu.com/p/25134841)

```cpp
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
```

---

## 跨平台

### QT共享内存

```CPP
#include "dialog.h"

#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSharedMemory share("forbid_repeat_start");
    if(!share.create(1)) {
        QMessageBox::information(NULL, "提示", "禁止程序重复启动", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return -1;
    }

    Dialog w;
    w.show();
    return a.exec();
}

```

### QT文件锁

```cpp
#include "dialog.h"

#include <QApplication>
#include <QLockFile>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QLockFile lock("./test.lock");
    if(!lock.tryLock(1)) {
        QMessageBox::information(NULL,"提示","禁止重复启动程序",QMessageBox::Ok,QMessageBox::Ok);
        return -1;
    }

    Dialog w;
    w.show();
    return a.exec();
}
```
