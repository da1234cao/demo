#include "main.h"

PDEVICE_OBJECT g_pDeviceObj = NULL;

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
    case ADD_RULE: { // 向驱动链表中添加规则
			if (inlen + 1 >= BUFFER_SIZE) { // 自定义的缓冲区长度不够保存输入进来的字符串
				nStatus = STATUS_UNSUCCESSFUL;
				break;
			}
      // 这里直接尝试将内容添加进规则。加一个校验，或许会好些
      add_rule(p_buffer, inlen);
		  kd_log_debug(("add rule: %s", p_buffer));
      break;
    }
    case CLEAR_RULE_LIST: { // 清空聊表规则
      clear_rules();
		  break; // 忘记写break.导致用户层总是收不到内容,排查了好久
    }
    case GET_RULE_LIST: { // 获取规则链表中的内容
      if (get_rule_list(p_buffer, &outlen) == TRUE) {
        ret_len = outlen;
      }
      else {
        nStatus = STATUS_UNSUCCESSFUL;
      }
      break;
    }
    default: {
      nStatus = STATUS_UNSUCCESSFUL;
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
  return g_pDeviceObj;
  kd_log_debug(("debug: create device."));
}

VOID DeleteDevice() {
  if (g_pDeviceObj != NULL) {
    kd_log_debug(("delete device."));
	  UNICODE_STRING uSymbolName;
	  RtlInitUnicodeString(&uSymbolName, DRIVER_OBJECT_SYM_NAME);
	  IoDeleteSymbolicLink(&uSymbolName);
	  IoDeleteDevice(g_pDeviceObj);
  }
}

VOID DriverUnload(PDRIVER_OBJECT driver)
{
	kd_log_debug(("debug: Our driver is unloading."));
  uninit_wfp();
  clear_rules();
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
  init_rule_list();

  init_wfp(g_pDeviceObj);

	driver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}