#include "device.h"
#include "list.h"
#include <ntddk.h>
#include <wdf.h>

VOID driver_unload(IN WDFDRIVER driver) {
	UNREFERENCED_PARAMETER(driver);
	clear_list();
	KdPrint(("wdf-demo: driver unload."));
}

extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT driver_obj, IN PUNICODE_STRING reg_path) {
	// 创建WDF驱动程序对象
	WDF_DRIVER_CONFIG config;
	WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK); // 非pnp驱动，不必提供EvtDriverDeviceAdd
	config.DriverInitFlags |= WdfDriverInitNonPnpDriver;
	config.EvtDriverUnload = driver_unload;
	WDFDRIVER driver;
	NTSTATUS status = WdfDriverCreate(driver_obj, reg_path, WDF_NO_OBJECT_ATTRIBUTES, &config, &driver);
	if (!NT_SUCCESS(status)) {
		KdPrint(("wdf-demo: fail in WdfDriverCreate."));
		return status;
	}
	
	WDFDEVICE device;
	status = device_init(driver, device);

	init_list(); // 初始化存储数据的双向链表

	return status;
}