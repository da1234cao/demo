#include "log.h"
#include<ntddk.h>

VOID DriverUnload(PDRIVER_OBJECT driver)
{
  log_debug("test: Our driver is unloading.");
  log_exit();
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path)
{
  wchar_t log_path[] = L"\\??\\C:\\test.txt";
  log_init(log_path);
  kd_log_debug(("kd_log_debug, enter in test driver: %wZ", reg_path));
  kd_log_debug_safe(("kd_log_debug_safe, enter in test driver: %wZ", reg_path));
  log_debug("log_debug, enter in test driver: %wZ", reg_path);
  log_debug_safe("log_debug_safe, enter in test driver: %wZ", reg_path);
  driver->DriverUnload = DriverUnload;
  return STATUS_SUCCESS;
}