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
	// ����WDF�����������
	WDF_DRIVER_CONFIG config;
	WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK); // ��pnp�����������ṩEvtDriverDeviceAdd
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

	init_list(); // ��ʼ���洢���ݵ�˫������

	return status;
}