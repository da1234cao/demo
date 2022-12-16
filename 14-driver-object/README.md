[toc]



## 前言

本文源码见仓库。

在网络编程中，入门的一个demo是，回射服务器。即，客户端发送数据给服务端，服务端再原样返回，可见：[chapter05_TCP客户_服务器示例_大1234草的博客-CSDN博客](https://da1234cao.blog.csdn.net/article/details/105069496)

在编写windows驱动的时候，用户层和内核层的通信必不可少(胡扯-…-)。本文实现，用户层发送数据给内核驱动->内核驱动保存数据->用户层再原样从内核驱动中将数据读取出来。

这套流程下来，我们会了解这些概念：驱动对象，设备对象，派遣函数，缓存方式，控制码。

实现本文demo的前置要求：

* [windows驱动开发环境搭建以及helloworld_大1234草的博客-CSDN博客_驱动环境搭建](https://da1234cao.blog.csdn.net/article/details/127933589)

* [windows内核编程-文件操作_大1234草的博客-CSDN博客](https://da1234cao.blog.csdn.net/article/details/128159702)

本文参考：

* 《windows内核安全与驱动开发》2.3 重要的数据结构、 第5章 应用与内核通信

* [64位内核开发第一讲,IRP 派遣函数 与 通信。 驱动框架补充 - iBinary - 博客园](https://www.cnblogs.com/iBinary/p/15838812.html)

* 参考代码：[source/coworker](https://github.com/dybb8999/Windows-kernel-security-and-driver-development-CD/tree/master/source/coworker)、[source/coworker2](https://github.com/dybb8999/Windows-kernel-security-and-driver-development-CD/tree/master/source/coworker2)

---

## 背景知识点介绍

### 驱动对象

相关阅读：

* 《windows内核安全与驱动开发》2.3.1 驱动对象

* [驱动程序对象简介 - Windows drivers | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/kernel/introduction-to-driver-objects)

* [《Windows驱动开发技术详解》之驱动程序的基本结构 - _No.47 - 博客园](https://www.cnblogs.com/predator-wang/p/5516418.html)

我不清楚内核中驱动对象的具体情况。我目前只用到它来设置派遣函数和卸载函数。

(下面搬运点内容。)一个驱动对象代表了一个驱动程序，或者说一个内核模块。如果写一个驱动程序，或者说编写一个内核模块，要在 Windows中加载，则必须填写这样一个结构，来告诉Windows程序提供哪些功能。驱动对象用[DRIVER_OBJECT](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/ddi/wdm/ns-wdm-_driver_object)数据结构表示。

---

### 设备对象与符号连接

相关阅读：

* 《windows内核安全与驱动开发》2.3.2 设备对象、5.1.2 控制设备的名字和符号链接、5.1.3 控制设备的删除

* [1.4 设备对象 与符号链接](https://www.cnblogs.com/iBinary/p/15838812.html#14-%E8%AE%BE%E5%A4%87%E5%AF%B9%E8%B1%A1-%E4%B8%8E%E7%AC%A6%E5%8F%B7%E9%93%BE%E6%8E%A5)

* [Windows驱动开发-符号链接和设备名 - AGB - 博客园](https://www.cnblogs.com/a-s-m/p/12329836.html)

* [设备对象简介 - Windows drivers | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/kernel/introduction-to-device-objects)

而在内核世界里，大部分“消息”都以请求(IRP)的方式传递。 而设备对象(DEVICE_OBJECT)是唯一可以接收请求的实体，任何一个 “请求”(IRP)都是发送给某个设备对象的。设备对象的结构是DEVICE_OBJECT。我们总是在内核程序中生成一个DEVICE_OBJECT，而一个内核程序是用一个驱动对象表示的，所以一个设备对象总是属于一个驱动对象。

设备对象是可以没有名字的。但是控制设备需要有一个名字，这样它才会被暴露出来，供其他程序打开与之通信。设备的名字可以在调用IoCreateDevice或IoCreateDeviceSecure时指定。此外，应用层是无法直接通过设备的名字来打开对象的，为此必须要建立一个暴露给应用层的符号链接。符号链接就是记录一个字符串对应到另一个字符串的一种简单结构。

> 在内核模式下，符号链接是以`“\DosDevices\`开头的，如C盘就是`"\??\C:"`，
> 
> 在用户模式下，符号链接是以`“\\.\”`开头的，如C盘就是`"\\.\C:"`.

下面，我们看下具体的代码实现。

```c
#define DRIVER_OBJECT_DEVICE_NAME L"\\Device\\object_device"

#define DRIVER_OBJECT_SYM_NAME L"\\DosDevices\\object_device" 

PDEVICE_OBJECT g_pDeviceObj = NULL;

PDEVICE_OBJECT CreateDevice(IN PDRIVER_OBJECT driver) {
  UNICODE_STRING uDeviceName = { 0 };
  UNICODE_STRING uSymbolNmae = { 0 };
  RtlInitUnicodeString(&uDeviceName, DRIVER_OBJECT_DEVICE_NAME);
  RtlInitUnicodeString(&uSymbolNmae, DRIVER_OBJECT_SYM_NAME);
  IoCreateDevice(driver, 0, &uDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &g_pDeviceObj);
  if (g_pDeviceObj != NULL) {
    g_pDeviceObj->Flags |= DO_BUFFERED_IO; // 缓冲区模式
  }
  IoCreateSymbolicLink(&uSymbolNmae, &uDeviceName);
}

VOID DeleteDevice() {
	if (g_pDeviceObj != NULL) {
	  UNICODE_STRING uSymbolName;
	  RtlInitUnicodeString(&uSymbolName, DRIVER_OBJECT_SYM_NAME);
	  IoDeleteSymbolicLink(&uSymbolName);
    IoDeleteDevice(g_pDeviceObj);
  }
}
```

---

### IRP和派遣函数

相关阅读：

* 《windows内核安全与驱动开发》2.3.3 请求

* [Windows驱动之IRP结构_xdesk的博客-CSDN博客_irp结构](https://blog.csdn.net/xiangbaohui/article/details/104910607)

* [IRP 与 派遣函数 - I_am - 博客园](https://www.cnblogs.com/lfls128/p/4978802.html)

* [64位内核开发第一讲,IRP 派遣函数 与 通信。 驱动框架补充 - iBinary - 博客园](https://www.cnblogs.com/iBinary/p/15838812.html#16-%E6%B4%BE%E9%81%A3%E5%87%BD%E6%95%B0%E4%B8%AD%E7%9A%84irp%E5%A4%84%E7%90%86)

* [IRP 主要函数代码 - Windows drivers | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/kernel/irp-major-function-codes)

搞不懂，但不妨碍写代码实现功能。把它当成钩子函数就好，特定的事件触发特定的函数。

完整示例代码，见后面的“实现源码”。

---

### 缓冲方式

相关阅读：

* [内核与应用层的通信方式-缓存方式缓冲区方式](https://www.cnblogs.com/iBinary/p/15838812.html)

* [访问数据缓冲区的方法 - Windows drivers | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/kernel/methods-for-accessing-data-buffers)

本文仅仅使用缓冲IO，其他方式不清楚。

操作系统会创建一个未分页的系统缓冲区，大小等于应用程序的缓冲区。 **对于写入操作，i/o 管理器会在调用驱动程序堆栈之前将用户数据复制到系统缓冲区。 对于读取操作，当驱动程序堆栈完成请求的操作后，i/o 管理器会将数据从系统缓冲区复制到应用程序的缓冲区中**。

缓冲IO 在我们创建完设备对象之后。将设备对象的标志设置为 `DO+_BUFFERD_IO`

如果设置为缓冲区模式。那么我们只需要在 `IRP结构` 中获取`AssociatedIrp.SystemBuffer` 即可。输入/输出缓冲区均使用这个缓冲区。

---

### 控制码

相关阅读：

* [5.2 控制码详解](https://www.cnblogs.com/iBinary/p/15838812.html#%E4%BA%94%E4%B8%B6io%E6%8E%A7%E5%88%B6%E8%AE%BE%E5%A4%87%E9%80%9A%E8%AE%AF%E6%96%B9%E5%BC%8F)

* [I/O 控制代码简介 - Windows drivers | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/kernel/introduction-to-i-o-control-codes)

* [定义 I/O 控制代码 - Windows drivers | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/kernel/defining-i-o-control-codes)

> I/o 控制代码 (IOCTLs) 用于用户模式应用程序和驱动程序之间的通信，或用于在堆栈中的驱动程序内部进行通信。 I/o 控制代码是使用 Irp 发送的。

---

## 实现源码

### 内核驱动主要代码

```c
#pragma once

#include "common/log.h"
#include <ntddk.h>

#define DRIVER_OBJECT_DEVICE_NAME L"\\Device\\object_device"

#define DRIVER_OBJECT_SYM_NAME L"\\DosDevices\\object_device"

#define IOCTL_OBJECT_TEST_REFLECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)

#define IOCTL_OBJECT_TEST_READ CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)

#define BUFFER_SIZE 1024
```

```c
#include "main.h"

PDEVICE_OBJECT g_pDeviceObj = NULL;
CHAR g_buffer[BUFFER_SIZE] = { 0 };

NTSTATUS IOControlDispatch(IN DEVICE_OBJECT* driver, IN PIRP irp) {
  NTSTATUS nStatus = STATUS_SUCCESS;
  ULONG ret_len = 0;
  UNREFERENCED_PARAMETER(driver);

  // irq结构中的缓冲区
  PVOID p_buffer = irp->AssociatedIrp.SystemBuffer;

  // irq当前使用的堆栈
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);

  // 缓冲区长度
  ULONG inlen = stack->Parameters.DeviceIoControl.InputBufferLength;
  ULONG outlen = stack->Parameters.DeviceIoControl.OutputBufferLength;
  kd_log_debug(("in buffer size: %lu", inlen));
  kd_log_debug(("out buffer size: %lu", outlen));

  switch (stack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_OBJECT_TEST_REFLECT: {
			if (inlen + 1 >= BUFFER_SIZE) { // 自定义的缓冲区长度不够保存输入进来的字符串
				nStatus = STATUS_INVALID_PARAMETER;
				break;
			}
			memcpy(g_buffer, p_buffer, BUFFER_SIZE);
			g_buffer[inlen] = '\0';
		  kd_log_debug(("send message: %s", g_buffer));
		  kd_log_debug(("send message len: %lu", strlen(g_buffer)));
      break;
    }
    case IOCTL_OBJECT_TEST_READ: {
			if (outlen < strlen(g_buffer) + 1) { // 这个输出缓冲区的大小，等于DeviceIoControl中nOutBufferSize参数大小
				nStatus = STATUS_INVALID_PARAMETER;
				break;
			}
			memcpy(p_buffer, g_buffer, outlen);
			*((char*)p_buffer + strlen(g_buffer)) = '\0';
			ret_len = strlen(p_buffer);
	    kd_log_debug(("read message: %s", p_buffer));
		  kd_log_debug(("read len: %lu", ret_len));
		  break; // 忘记写break.导致用户层总是收不到内容,排查了好久
    }
    default: {
      nStatus = STATUS_INVALID_PARAMETER;
    }
  }

  irp->IoStatus.Information = ret_len;
  irp->IoStatus.Status = nStatus;
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}

NTSTATUS IONothingDispatch(IN DEVICE_OBJECT* driver, IN PIRP irp) {
  UNREFERENCED_PARAMETER(driver);
  NTSTATUS n_status = STATUS_SUCCESS;
  irp->IoStatus.Information = 0;
  irp->IoStatus.Status = n_status;
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  return n_status;
}

PDEVICE_OBJECT CreateDevice(IN PDRIVER_OBJECT driver) {
  UNICODE_STRING uDeviceName = { 0 };
  UNICODE_STRING uSymbolNmae = { 0 };
  RtlInitUnicodeString(&uDeviceName, DRIVER_OBJECT_DEVICE_NAME);
  RtlInitUnicodeString(&uSymbolNmae, DRIVER_OBJECT_SYM_NAME);
  IoCreateDevice(driver, 0, &uDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &g_pDeviceObj);
  if (g_pDeviceObj != NULL) {
    g_pDeviceObj->Flags |= DO_BUFFERED_IO; // 缓冲区模式
  }
  IoCreateSymbolicLink(&uSymbolNmae, &uDeviceName);
}

VOID DeleteDevice() {
	if (g_pDeviceObj != NULL) {
	  UNICODE_STRING uSymbolName;
	  RtlInitUnicodeString(&uSymbolName, DRIVER_OBJECT_SYM_NAME);
	  IoDeleteSymbolicLink(&uSymbolName);
    IoDeleteDevice(g_pDeviceObj);
  }
}

VOID DriverUnload(PDRIVER_OBJECT driver)
{
	kd_log_debug(("test: Our driver is unloading."));
  DeleteDevice();
	log_exit();
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path)
{
	UNREFERENCED_PARAMETER(reg_path);

	wchar_t log_path[] = L"\\??\\C:\\test.txt";
	log_init(log_path);
	kd_log_debug(("enter driver object test: %wZ", reg_path));

  driver->MajorFunction[IRP_MJ_CREATE] = IONothingDispatch;
  driver->MajorFunction[IRP_MJ_CLOSE] = IONothingDispatch;
  driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IOControlDispatch;

  CreateDevice(driver);

	driver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}
```

### 用户层代码

```cpp
#pragma once

#include <Windows.h>
#include <iostream>
#include <string>

#define DRIVER_OBJECT_SYM_NAME "\\\\.\\object_device"

#define IOCTL_OBJECT_TEST_REFLECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)

#define IOCTL_OBJECT_TEST_READ CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)
```

```cpp
#include "main.h"

int main(int argc, char* argv[]) {
  HANDLE device = CreateFile(DRIVER_OBJECT_SYM_NAME, GENERIC_READ | GENERIC_WRITE,
                             0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);                           
  if(device == INVALID_HANDLE_VALUE) {
    std::cerr << "fail to open device" << std::endl;
    return -1;
  }
  
  std::string msg = "hello,world";
  unsigned long ret_len = 0;
  BOOL result = DeviceIoControl(device, IOCTL_OBJECT_TEST_REFLECT, (LPVOID)msg.c_str(), msg.size(), NULL, 0, (LPDWORD)&ret_len, NULL);
  if(result == FALSE) {
    std::cerr << "fail in DeviceIoControl: IOCTL_OBJECT_TEST_REFLECT" << std::endl;
    return -1;
  }

  char read_buff[1024] = {0};
  ret_len = 1024;
  result = DeviceIoControl(device, IOCTL_OBJECT_TEST_READ, NULL, 0, read_buff, 1024, (LPDWORD)&ret_len, NULL);
  if(result == FALSE) {
    std::cerr << "fail in DeviceIoControl: IOCTL_OBJECT_TEST_READ" << std::endl;
    DWORD err_code = GetLastError();
    std::cerr << "err code: " << err_code << std::endl;
    return -1;
  }

  std::cout << "get content:" << std::string(read_buff) << std::endl;
  std::cout << "length: " << ret_len << std::endl;

  CloseHandle(device);
  return 0;
}
```


