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