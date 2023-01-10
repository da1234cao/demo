#include "io_callbacks.h"
#include "../libdemo/demo_common.h"
#include "list.h"

VOID file_create(IN WDFDEVICE device, IN WDFREQUEST request, IN WDFFILEOBJECT object) {
	UNREFERENCED_PARAMETER(device);
	UNREFERENCED_PARAMETER(object);
	KdPrint(("wdf-demo: file_create."));
	WdfRequestComplete(request, STATUS_SUCCESS);
}


VOID file_close(IN WDFFILEOBJECT object)
{
	KdPrint(("wdf-demo: file_close."));
	UNREFERENCED_PARAMETER(object);
}

VOID file_cleanup(IN WDFFILEOBJECT object)
{
	KdPrint(("wdf-demo: file_cleanup."));
	UNREFERENCED_PARAMETER(object);
}

VOID device_ioctl(IN WDFQUEUE queue, IN WDFREQUEST request,
	IN size_t out_length, IN size_t in_length, IN ULONG code)
{
	UNREFERENCED_PARAMETER(queue);
	UNREFERENCED_PARAMETER(out_length);
	UNREFERENCED_PARAMETER(in_length);

	NTSTATUS status;

	if (code == IOCTL_GET_MSG) {
		KdPrint(("wdf-demo:device_ioctl - IOCTL_GET_MSG"));
		void* outbuf;
		size_t outlen;
		status = WdfRequestRetrieveOutputBuffer(request, 0, &outbuf, &outlen);
		if (!NT_SUCCESS(status)) {
			KdPrint(("wdf-demo:device_ioctl - IOCTL_GET_MSG. get input buffer error: %d", status));
			WdfRequestComplete(request, STATUS_SUCCESS);
			return;
		}

		conn_item* node = remove_head();
		KdPrint(("wdf-demo:device_ioctl - IOCTL_SET_MSG. outbuf is %s", node->connect));
		if (node != NULL) {
			memcpy(outbuf, &node->connect, outlen);
			ExFreePoolWithTag(node, WDF_DEMO_TAG);
			WdfRequestCompleteWithInformation(request, status, strlen((char*)outbuf));
		}
		else {
			WdfRequestComplete(request, STATUS_SUCCESS);
		}
		return;
	}
	else if (code == IOCTL_SET_MSG) {
		KdPrint(("wdf-demo:device_ioctl - IOCTL_SET_MSG"));
		void *inbuf;
		size_t inlen;
		status = WdfRequestRetrieveInputBuffer(request, 0, &inbuf, &inlen);
		if (!NT_SUCCESS(status)) {
			KdPrint(("wdf-demo:device_ioctl - IOCTL_SET_MSG. get input buffer error: %d", status));
			WdfRequestComplete(request, STATUS_SUCCESS);
			return;
		}
		*((char*)inbuf + inlen) = '\0';
		KdPrint(("wdf-demo:device_ioctl - IOCTL_SET_MSG. in buf is %s", (char*)inbuf));
		add_tail(inbuf, inlen);
		WdfRequestComplete(request, STATUS_SUCCESS);
		return;
	}
	else {
		KdPrint(("wdf-demo:device_ioctl - UNKNOWS"));
		WdfRequestComplete(request, STATUS_UNSUCCESSFUL);
	}
}