#include "device.h"

NTSTATUS device_init(WDFDRIVER& driver, WDFDEVICE& device) {
	DECLARE_CONST_UNICODE_STRING(device_name, L"\\Device\\wdf_demo");
	DECLARE_CONST_UNICODE_STRING(dos_device_name, L"\\??\\wdf_demo");

	NTSTATUS status = STATUS_SUCCESS;

	// 创建一个设备对象

	//// 设置设备类型，设备缓冲模式，设备名
	PWDFDEVICE_INIT device_init = WdfControlDeviceInitAllocate(driver, &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);
	if (device_init == NULL) {
		status = STATUS_INSUFFICIENT_RESOURCES;
		KdPrint(("wdf-demo: fail in WdfControlDeviceInitAllocate."));
		return status;
	}
	WdfDeviceInitSetDeviceType(device_init, FILE_DEVICE_UNKNOWN);
	WdfDeviceInitSetIoType(device_init, WdfDeviceIoBuffered); // 缓冲模式，类似DO_BUFFERED_IO
	WdfDeviceInitAssignName(device_init, &device_name);

	//// 设置设备创建和关闭的回调函数
	WDF_FILEOBJECT_CONFIG file_config;
	WDF_FILEOBJECT_CONFIG_INIT(&file_config, file_create, file_close, file_cleanup);
	WdfDeviceInitSetFileObjectConfig(device_init, &file_config, WDF_NO_OBJECT_ATTRIBUTES);

	status = WdfDeviceCreate(&device_init, WDF_NO_OBJECT_ATTRIBUTES, &device);
	if (!NT_SUCCESS(status)) {
		KdPrint(("wdf-demo: fail in WdfDeviceCreate, ret status is %d", status));
		// WdfControlDeviceInitAllocate得到的WDFDEVICE_INIT 结构，在驱动程序初始化错误的时候必须调用 WdfDeviceInitFree
		// 成功调用 WdfDeviceCreate 后，驱动程序不得调用 WdfDeviceInitFree
		WdfDeviceInitFree(device_init);
		return status;
	}

	//// 该设备的IO操作
	WDF_IO_QUEUE_CONFIG queue_config;
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queue_config, WdfIoQueueDispatchSequential); // 请求串行处理
	queue_config.EvtIoDeviceControl = device_ioctl; // 请求的处理函数
	WDFQUEUE queue;
	status = WdfIoQueueCreate(device, &queue_config, WDF_NO_OBJECT_ATTRIBUTES, &queue);
	if (!NT_SUCCESS(status)) {
		KdPrint(("wdf-demo: fail in WdfIoQueueCreate, ret status is %d", status));
		return status;
	}

	// 创建设备的符号链接
	status = WdfDeviceCreateSymbolicLink(device, &dos_device_name);
	if (!NT_SUCCESS(status)) {
		KdPrint(("wdf-demo: fail in WdfDeviceCreateSymbolicLink, ret status is %d", status));
		return status;
	}

	WdfControlFinishInitializing(device);

	return status;
}