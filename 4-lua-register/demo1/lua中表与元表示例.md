## 前言

这篇文章介绍lua的表和元表。比较简单，主要为后面的userdata做准备。

相关链接：
1. [【Lua】元表、元方法、面向对象 - BiliBili](https://www.bilibili.com/video/BV1f44y1a7Gk/)
2. [Lua 元表 - 中文手册](https://www.itbook.team/book/lua/LUAJiaoCheng/LUAYuanBiaoMETATABLE.html)、[lua中神奇的table](http://blog.lujun9972.win/blog/2018/06/17/lua%E4%B8%AD%E7%A5%9E%E5%A5%87%E7%9A%84table/index.html)
3. 《lua程序设计》第四版： 第五章表、第二十章 元表和元方法
4. [Lua 实现复数计算器](https://blog.csdn.net/z2014z/article/details/118765371)

上面链接过一遍，基本上能搞清楚lua的表和元表。下面画蛇添足的复制上面的部分内容，作为笔记。

这篇博客的诞生过程是：看代码，了解使用->找博客，去解决代码中不懂的部分->改代码，以满足需求->翻书，补充下概念点->最后是来写demo，做个笔记。

本文不会一点点的去复制概念，因为无聊。本文给出demo，然后摘录一些点。

---

## lua表简介

lua中表的demo代码。

```lua
local tab = {}
tab[1] = 111
tab.second = "222"
tab["third"] = "333"
function tab.hello() 
  print("hello world")
end

for k, v in pairs(tab) do
  if(type(v) == "number") then
    print(v)
  elseif(type(v) == "string") then
    print(v)
  elseif(type(v) == "function") then
    v()
  end
end
```

运行输出如下。

```shell
111
hello world
222
333
```

下面简单复制下，lua中table的一些小点。

* 表（Table）是Lua语言中最主要（事实上也是唯一的）和强大的数据结构。表是一种动态分配的对象，程序只能操作指向表的引用（或指针）。除此以外，**Lua语言不会进行隐藏的拷贝（hidden copies）或创建新的表**。
* 我们使用构造器表达式（ constructor expression ）创建表，其最简单的形式是{}。对于一个表而言，当程序中不再有指向它的引用时，垃圾收集器会最终删除这个表并重用其占用的内存。
* **同一个表中存储的值可以具有不同的类型索引**。
* 当把表当作结构体使用时，可以把索引当作成员名称使用（**a.name等价于a["name"]**）。这两种形式可能代表了不同的意图。形如a.name的点分形式清晰地说明了表是被当作结构体使用的，此时表实际上是由固定的、预先定义的键组成的集合；而形如a["name"]的字符串索引形式则说明了表可以使用任意字符串作为键，并且出于某种原因我们操作的是指定的键。
* C#的一些编程语言提供了一种安全访问操作符（ safe navigation operator ）。在C#中，这种安全访问操作符被记为“？.”。例如，对于表达式a？.b，当a为nil时，其结果是nil而不会产生异常。对于表达式a or{}，当a为nil时其结果是一个空表。因此，对于表达式（a or{}）.b，当a为nil时其结果也同样是nil。

## lua元表简介

面向对象编程，对象包含属性和方法。面向对象中对象的方法 $\approx$ lua元表。

通常，Lua语言中的每种类型的值都有一套可预见的操作集合。元表可以修改一个值在面对一个未知操作时的行为。例如，假设a和b都是表，那么可以通过元表定义Lua语言如何计算表达式a+b。当Lua语言试图将两个表相加时，它会先检查两者之一是否有元表（metatable ）且该元表中是否有__add字段。如果Lua语言找到了该字段，就调用该字段对应的值，即所谓的元方法（ metamethod ）。

**在Lua语言中，我们只能为表设置元表；如果要为其他类型的值设置元表，则必须通过C代码或调试库完成**。（为userdate设置元表，是我想整理的内容。所以整理了这篇作为铺垫。）

* 运算相关的元方法：每种算术运算符都有一个对应的元方法。除了加法和乘法外，还有减法（__sub）、除法（__div）、floor除法（__idiv）、负数（__unm）、取模（__mod）和幂运算（__pow）。类似地，位操作也有元方法 ：按位与（ __band ）、按位或（ __bor  、按位异或
（__bxor）、按位取反（__bnot）、向左移位（__shl）和向右移位（__shr）。我们还可以使用字段__concat来定义连接运算符的行为。

* 关系运算符相关的元方法：元表还允许我们指定关系运算符的含义，其中的元方法包括等于（__eq）、小于（__lt）和小于等于（__le）。其他三个关系运算符没有单独的元方法，Lua语言会将a～=b转换为not（a==b），a>b转换为b<a，a>=b转换为b<=a。

* 库定义相关的元方法：函数print总是调用tostring来进行格式化输出。不过，当对值进行格式化时，函数tostring会首先检查值是否有一个元方法__tostring。

* **表相关的元方法**
  * `__index` 元方法： 当你通过键来访问 table 的时候，如果这个键没有值，那么Lua就会寻找该table的metatable（假定有metatable）中的__index 键。如果__index包含一个表格，Lua会在表格中查找相应的键。
  * 元方法__newindex与__index类似，不同之处在于前者用于表的更
新而后者用于表的查询。当对一个表中不存在的索引赋值时，解释器
就会查找__newindex元方法：如果这个元方法存在，那么解释器就调
用它而不执行赋值。




