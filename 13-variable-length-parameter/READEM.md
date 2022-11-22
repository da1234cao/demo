[toc]

## 前言

详细见下面内容，这里简单搬运下。
1. 《C语言程序设计现代方法》26.1 可变参数
2. 《C++ primer》16.4 可变参数模板
3. [va_start -- cppreference](https://zh.cppreference.com/w/c/variadic/va_start)
4. [【C++编程之杂项笔记】1: 两种变长参数函数比较](https://elloop.github.io/c++/2015-11-28/never-proficient-cpp-vaargs)

---
## va_arg - 变长参数函数

printf 和scanf 这样的函数具有一个不同寻常的性质：它们允许任意数量的参数。而且，这种能处理可变数量的参数的能力并不仅限于库函数。<stdarg.h> 头提供的工具使我们能够自己编写带有变长参数列表的函数。<stdarg.h> 声明了一种类型（va_list ）并定义了几个宏。C89中一共有三个宏，分别名为va_satrt、va_arg和va_end。 C99增加了一个类似函数的宏va_copy 

下面函数用来在任意数量的整数参数中找出最大数。

```c
#include <stdio.h>
#include <stdarg.h>

int max_int(int n, ...) {
  va_list ap;
  int i, current, largest;

  va_start(ap, n);
  largest = va_arg(ap, int);

  for (i = 0; i < n; i++) {
    current = va_arg(ap, int);
    if (current > largest)
    largest = current;
  }
  
  va_end(ap);
  return largest;
}

int main(int argc, char* argv[])
{
  int max_num = max_int(3, 10, 30, 20);
  printf("%d\n", max_num);
}
```

* 在形式参数列表中的... 符号（省略号）表示参数n 后面有可变数量的参数。
* `va_list ap;`声明va_list 类型的变量。
* `va_start(ap, n);`指出了参数列表中可变长度部分开始的位置（这里从n 后边开始）。带有可变数量参数的函数必须至少有一个“正常的”形式参数；省略号总是出现在形式参数列表的末尾，在最后一个正常参数的后边。(C23中并非如此)
* `va_arg(ap, int);`(逐个)获取函数余下的参数。
* ` va_end (ap);`进行“清理”

---
## va_arg - 格式化字符串输出

带有可变参数列表的函数无法确定参数的数量和类型。这一信息必须被传递给函数或者由函数来假定。示例中的max_int 函数依靠第一个参数来指明后面有多少参数，并且它假定参数都是int 类型的。而像printf 和scanf 这样的函数则是依靠格式串来描述其他参数的数量以及每个参数的类型。

另外一个问题是关于以NULL 作为参数的。NULL 通常用于表示0 。当把0 作为参数传递给带有可变参数列表的函数时，编译器会假定它表示一个整数——无法用于表示空指针。解决这一问题的方法就是添加一个强制类型转换，用(void *)NULL 或(void *)0 来代替NULL 。

v…printf系列的函数使用，参见：[std::vprintf, std::vfprintf, std::vsprintf, std::vsnprintf](https://zh.cppreference.com/w/cpp/io/c/vfprintf)

```c
#include <stdio.h>
#include <stdarg.h>

void print_log(char * format, ...) {
  va_list va;
  va_start(va, format);
  char buffer[256]; // 小心缓冲区溢出
  vsprintf(buffer, format, va);
  va_end(va);

  printf("%s\n", buffer);
}

int main(int argc, char* argv[])
{
  print_log("variable length parameter:%d-%s", 1, "abc");
  return 0;
}
```

---
## 可变参数模板

(复制粘贴很麻烦，这里简单复制下，自行翻看《C++ primer》)

一个可变参数模板（variadic template）就是一个接受可变数目参数的模板函数或模板类。可变数目的参数被称为参数包（parameter packet）。存在两种参数包：模板参数包（template parameterpacket），表示零个或多个模板参数；函数参数包（function parameter packet），表示零个或多个函数参数。

可变参数函数通常是递归的。以下面程序为例。第一步调用处理包中的第一个实参，然后用剩余实参调用自身。我们的print函数也是这样的模式，每次递归调用将第二个实参打印到第一个实参表示的流中。为了终止递归，我们还需要定义一个非可变参数的print函数，它接受一个流和一个对象：第一个版本的print负责终止递归并打印初始调用中的最后一个实参。第二个版本的print是可变参数版本，它打印绑定到t的实参，并调用自身来打印函数参数包中的剩余值。

对于最后一次递归调用，两个print版本都是可行的。对于最后一个调用，两个函数提供同样好的匹配。但是，非可变参数模板比可变参数模板更特例化，因此编译器选择非可变参数版本。

```cpp
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
```
