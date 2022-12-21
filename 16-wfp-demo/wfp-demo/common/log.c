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
