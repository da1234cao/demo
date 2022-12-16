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