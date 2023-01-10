#include "device.h"

NTSTATUS device_init(WDFDRIVER& driver, WDFDEVICE& device) {
	DECLARE_CONST_UNICODE_STRING(device_name, L"\\Device\\wdf_demo");
	DECLARE_CONST_UNICODE_STRING(dos_device_name, L"\\??\\wdf_demo");

	NTSTATUS status = STATUS_SUCCESS;

	// ����һ���豸����

	//// �����豸���ͣ��豸����ģʽ���豸��
	PWDFDEVICE_INIT device_init = WdfControlDeviceInitAllocate(driver, &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);
	if (device_init == NULL) {
		status = STATUS_INSUFFICIENT_RESOURCES;
		KdPrint(("wdf-demo: fail in WdfControlDeviceInitAllocate."));
		return status;
	}
	WdfDeviceInitSetDeviceType(device_init, FILE_DEVICE_UNKNOWN);
	WdfDeviceInitSetIoType(device_init, WdfDeviceIoBuffered); // ����ģʽ������DO_BUFFERED_IO
	WdfDeviceInitAssignName(device_init, &device_name);

	//// �����豸�����͹رյĻص�����
	WDF_FILEOBJECT_CONFIG file_config;
	WDF_FILEOBJECT_CONFIG_INIT(&file_config, file_create, file_close, file_cleanup);
	WdfDeviceInitSetFileObjectConfig(device_init, &file_config, WDF_NO_OBJECT_ATTRIBUTES);

	status = WdfDeviceCreate(&device_init, WDF_NO_OBJECT_ATTRIBUTES, &device);
	if (!NT_SUCCESS(status)) {
		KdPrint(("wdf-demo: fail in WdfDeviceCreate, ret status is %d", status));
		// WdfControlDeviceInitAllocate�õ���WDFDEVICE_INIT �ṹ�������������ʼ�������ʱ�������� WdfDeviceInitFree
		// �ɹ����� WdfDeviceCreate ���������򲻵õ��� WdfDeviceInitFree
		WdfDeviceInitFree(device_init);
		return status;
	}

	//// ���豸��IO����
	WDF_IO_QUEUE_CONFIG queue_config;
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queue_config, WdfIoQueueDispatchSequential); // �����д���
	queue_config.EvtIoDeviceControl = device_ioctl; // ����Ĵ�����
	WDFQUEUE queue;
	status = WdfIoQueueCreate(device, &queue_config, WDF_NO_OBJECT_ATTRIBUTES, &queue);
	if (!NT_SUCCESS(status)) {
		KdPrint(("wdf-demo: fail in WdfIoQueueCreate, ret status is %d", status));
		return status;
	}

	// �����豸�ķ�������
	status = WdfDeviceCreateSymbolicLink(device, &dos_device_name);
	if (!NT_SUCCESS(status)) {
		KdPrint(("wdf-demo: fail in WdfDeviceCreateSymbolicLink, ret status is %d", status));
		return status;
	}

	WdfControlFinishInitializing(device);

	return status;
}