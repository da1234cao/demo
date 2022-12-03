# 

# windows内核编程-文件操作

[toc]

## 前言

我们必须先搞定，`DebugView`可以看到`DbgPrintDbgPrint`的输出。否则，代码没法写。因为，大概率代码不会一次通过，需要结合输出信息进行修改。而这些准备工作见：[windows驱动开发环境搭建以及helloworld_大1234草的博客-CSDN博客_驱动环境搭建](https://blog.csdn.net/sinat_38816924/article/details/127933589)

在用户层编程，通常使用日志记录程序运行情况。我们把这个习惯延续到内核编程中来。

我上网上找了一圈，似乎没有很好的win内核编程日志框架，所以这里简单实现一个。这里假定我们已经知道用户层如何使用日志：[spdlog日志库的封装使用_大1234草的博客-CSDN博客_spdlog 封装](https://da1234cao.blog.csdn.net/article/details/126192561)

对这个内核日志的实现有如下要求：

* 每次打开日志文件的时候覆盖之前的日志。(为了避免日志文件大小一直增加。也避免日志文件循环这样有些复杂的实现。毕竟主要是调试的时候使用)

* 可以控制，在debug版本的时候显示日志，在release版本的时候，日志函数为空。

* 不提供日志等级。（日志等级的实现毫无难度，只是目前没有必要）

* 提供线程安全，和线程不安全两种日志记录函数。

---

## 背景知识点介绍

参考：《Windows内核安全与驱动开发》第二章 内核编程环境及其特殊性、第三章 字符串与链表、第四章 文件、注册表、线程

书上的内容翻下就好，不需要细看。初级编程，了解有哪些数据结构+API就好。

至于windows官方文档，它比较零碎，查某个具体的API可以，但是并没有将这些API串起来介绍。

---

### 字符串

参考：[_UNICODE_STRING (ntdef.h) - Win32 apps | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows/win32/api/ntdef/ns-ntdef-_unicode_string)、[Windows 内核驱动字符串操作 &#8211; My Code](https://www.mycode.net.cn/platform/1543.html)

使用**UNICODE_STRING**结构用于定义 Unicode 字符串。它的数据结构如下所示：

```c
typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
```

该数据结构相关的操作函数：

* 初始化函数：RtlInitUnicodeString

* 拷贝函数：RtlCopyUnicodeString

* Append 字符串：RtlAppendUnicodeToString

字符串相关：[Windows 内核模式安全字符串库 - Windows drivers | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/kernel/windows-kernel-mode-safe-string-library)

---

### 内存分配与回收

参考：[Windows内核编程基础之内存的分配与释放_PandaMohist的博客-CSDN博客](https://blog.csdn.net/HK_5788/article/details/48003063)

分配内存：[exAllocatePool](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/ddi/wdm/nf-wdm-exallocatepool)函数已经过时， 它已被 [ExAllocatePool2](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/ddi/wdm/nf-wdm-exallocatepool2) 取代。

释放内存：ExFreePool

---

### 文件操作

参考：[在驱动程序中使用文件 - Windows drivers | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/kernel/using-files-in-a-driver)、[Windows内核驱动中操作文件 （转） - 心如止水-杜 - 博客园](https://www.cnblogs.com/dhf327/p/4793674.html)、[Windows内核编程之：文件操作 - qintangtao - 博客园](https://www.cnblogs.com/qintangtao/archive/2013/05/08/3067240.html)

建议去翻下书，系统的了解下这套API的使用。这里不赘述。

---

### 自旋锁

锁可以解决线程同步问题。这里选用自旋锁(其他锁没用过，也不知道)

参考：[使用驱动程序提供的自旋锁 - Windows drivers | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/kernel/using-a-driver-supplied-spin-lock)

---

## win内核编程-日志实现

### 文件操作的封装

windows的函数参数多的令人脑子疼。

原有的API比较细粒度，控制更准确，但是调用起来会麻烦点。所以，习惯上会简单的封装下。

```c
#pragma once

/* modified from : aurain, https://www.cnblogs.com/dhf327/p/4793674.html  */

#include <ntddk.h>

/**/
/** * 创建或打开文件
* @param lpFileHandle 返回打开的文件句柄指针
* @param usFileName 需要打开的文件路径，使用对象路径，如\\??\\c:\test.txt
* @param dwDesiredAccess 申请权限，可以用|（或）组合以下操作
    写文件内容-FILE_WRITE_DATA，
    设置文件属性-FILE_WRITE_ATTRIBUTES，通用写-GENERIC_WRITE 读文件内容-FILE_READ_DATA，
    设置文件属性-FILE_READE_ATTRIBUTES，通用写-GENERIC_READ 删除文件-DELETE 全部权限-GENERIC_ALL 同步打开文件-SYNCHRONIZE
* @param dwShareAccess 共享方式（是指本代码打开这个文件时，允许别的代码同时打开这个文件所具有的权限 可以用|（或）组合以下操作
    共享读-FILE_SHARE_READ
    共享写-FILE_SHARE_WRITE
    共享删除-FILE_SHARE_DELETE
* @param dwCreateDisposition 创建或打开文件的目的
    新建文件-FILE_CREATE
    打开文件-FILE_OPEN
    打开或新建-FILE_OPEN_IF
    覆盖-FILE_OVERWRITE
    新建或覆盖-FILE_OVERWRITE_IF
    新建或取代-FILE_SUPERSEDE
* @param dwCreateOptions 打开文件时选项设置 一般用FILE_NOT_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT

* @return 读取成功： STATUS_SUCCESS；读取失败：NTSTATUS error code */

__inline NTSTATUS CreateFile(OUT PHANDLE lpFileHandle,
  IN PUNICODE_STRING usFileName,
  IN ULONG dwDesiredAccess,
  IN ULONG dwShareAccess,
  IN ULONG dwCreateDisposition,
  IN ULONG dwCreateOptions) {

  NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
  OBJECT_ATTRIBUTES oaName;
  IO_STATUS_BLOCK iosBlock;
  if (lpFileHandle != NULL && usFileName != NULL && usFileName->Buffer != NULL) {
    InitializeObjectAttributes(&oaName, usFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    ntStatus = ZwCreateFile(lpFileHandle, dwDesiredAccess, &oaName, &iosBlock, NULL, FILE_ATTRIBUTE_NORMAL, dwShareAccess, dwCreateDisposition, dwCreateOptions, NULL, 0);
  }
  return ntStatus;
}


/**/
/** * 关闭打开的文件句柄
* @param hFile 文件句柄
* @return 读取成功： STATUS_SUCCESS；读取失败：NTSTATUS error code
*/

__inline NTSTATUS CloseFile(IN HANDLE hFile) {
  return ZwClose(hFile);
}

/**/

/** * 读取文件内容
* @param hFile 文件句柄
* @param pBuffer 缓冲区
* @param ulBufferSize 缓冲区大小
* @param byteOffset 偏移量
* @param pulBytesRead 实际读取的大小
* @return 读取成功： STATUS_SUCCESS；读取失败：NTSTATUS error code
*/

__inline NTSTATUS ReadFile(IN HANDLE hFile, IN PVOID pBuffer, IN ULONG ulBufferSize, IN PLARGE_INTEGER byteOffset, OUT PULONG pulBytesRead) {
  IO_STATUS_BLOCK  iosBlock; 
  NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
  if (hFile == NULL || pBuffer == NULL) {
    return ntStatus;
  }
  *pulBytesRead = 0;
  ntStatus = ZwReadFile(hFile, NULL, NULL, NULL, &iosBlock, pBuffer, ulBufferSize, byteOffset, NULL);
  if (NT_SUCCESS(ntStatus)) {
    //获取实际读取到的大小
    *pulBytesRead = (ULONG)iosBlock.Information;
  }
  return ntStatus;
}

/**/
/** * 向文件写入内容
* @param hFile 文件句柄
* @param pBuffer 缓冲区
* @param ulBufferSize 缓冲区大小
* @param byteOffset 偏移量
* @return 读取成功： STATUS_SUCCESS；读取失败：NTSTATUS error code
*/
__inline NTSTATUS WriteFile(IN HANDLE hFile, IN PVOID pBuffer, IN ULONG ulBufferSize, IN PLARGE_INTEGER byteOffset, OUT PULONG pulBytesWrite) {
  IO_STATUS_BLOCK iosBlock; 
  NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
  if (hFile == NULL || pBuffer == NULL) {
    return ntStatus;
  }
  DbgPrint("before zwwritefile.");
  *pulBytesWrite = 0;
  ntStatus = ZwWriteFile(hFile, NULL, NULL, NULL, &iosBlock, pBuffer, ulBufferSize, byteOffset, NULL);
  if (NT_SUCCESS(ntStatus)) {
    *pulBytesWrite = (ULONG)iosBlock.Information;
  }
  return ntStatus;
}
```

### 日志功能的实现

上面封装的很浅。调用上面的接口，以将日志信息写入文件。

头文件的定义。

```c
#pragma once

#include "file_help.h"
#include <wdm.h>
#include <ntstrsafe.h>

#define BUFF_SIZE 256

int log_init(wchar_t* plog_file);

int log_exit();

int write_log(char* format, va_list arg, BOOLEAN thread_safe);

void log_debug(char* format, ...);

void log_debug_safe(char* format, ...);

#if DBG 
#define kd_log_debug(_x_) log_debug _x_
#define kd_log_debug_safe(_x_) log_debug_safe _x_
#else
#define kd_log_debug(_x_)
#define kd_log_debug_safe(_x_)
#endif
```

头文件的实现。

```c
#include "log.h"

HANDLE log_file_handle = NULL;
KSPIN_LOCK spin_lock;

int log_init(wchar_t* plog_file)
{
  // 初始化自旋锁
  KeInitializeSpinLock(&spin_lock);

  UNICODE_STRING unicode_log_file;
  RtlInitUnicodeString(&unicode_log_file, plog_file);
  // 具有读写权限,写追加。允许其他线程读取。每次打开进行覆盖，没有的话则创建。文件不是一个目录文件。
  // NTSTATUS ntStatus = CreateFile(log_file_handle, &unicode_log_file, FILE_READ_DATA | FILE_APPEND_DATA, FILE_SHARE_READ, FILE_OVERWRITE_IF, FILE_NON_DIRECTORY_FILE);
  // 注意最后一个参数。如果只设置成FILE_NON_DIRECTORY_FILE，而不设置FILE_SYNCHRONOUS_IO_NONALERT，在zwWritefile的时候，会返回STATUS_INVALID_PARAMETER
  NTSTATUS ntStatus = CreateFile(&log_file_handle, &unicode_log_file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_VALID_FLAGS, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT);
  DbgPrint("create log file status: %x", ntStatus);
  return 0;
}

int log_exit()
{
  if (log_file_handle != NULL) {
    CloseFile(log_file_handle);
  }
  return 0;
}

int write_log(char* format, va_list arg, BOOLEAN thread_safe)
{
  NTSTRSAFE_PSTR buffer = NULL; //
  buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, BUFF_SIZE, 'LOGM');
  if (buffer == NULL) {
    return -1;
  }
  RtlStringCbVPrintfA(buffer, BUFF_SIZE, format, arg);
  RtlStringCbCatA(buffer, BUFF_SIZE, "\r\n");
  size_t in_len;
  RtlStringCbLengthA(buffer, BUFF_SIZE, &in_len);
  ULONG write_len = 0;

  // DbgPrint("log write: %s", buffer);
  // DbgPrint("log size: %lld", in_len);
  KIRQL irql;
  if (thread_safe == TRUE) {
    KeAcquireSpinLock(&spin_lock, &irql);
  }
  NTSTATUS ntStatus = WriteFile(log_file_handle, buffer, in_len, NULL, &write_len);
  // DbgPrint("log write in: %ld", write_len);
  if (thread_safe == TRUE) {
    KeReleaseSpinLock(&spin_lock, irql);
  }

  DbgPrint("write log file status: %x", ntStatus);
  ExFreePool(buffer);
  return 0;
}

void log_debug(char* format, ...) {
  va_list va;
  va_start(va, format);
  write_log(format, va, FALSE);
  va_end(va);
}

void log_debug_safe(char* format, ...) {
  va_list va;
  va_start(va, format);
  write_log(format, va, TRUE);
  va_end(va);
}

```

### 简单的测试日志功能

```c

```
