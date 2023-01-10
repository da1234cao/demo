#include "list.h"

LIST_ENTRY connect_list;
WDFWAITLOCK connect_list_lock;

void init_list() {
	WdfWaitLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &connect_list_lock);
	InitializeListHead(&connect_list);
}

BOOLEAN add_tail(PVOID inbuf, size_t inlen) {
	if (inlen >= ENTER_MAX_SIZE) {
		return FALSE;
	}

	conn_item *node = (conn_item*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(conn_item), WDF_DEMO_TAG);
	if (node == NULL) {
		KdPrint(("fail in ExAllocatePool2"));
		return FALSE;
	}
	memset(node, 0, sizeof(node));
	memcpy(&node->connect, inbuf, ENTER_MAX_SIZE);

	KdPrint(("wdf-demo:add tail . node->connect is %s", node->connect));
	WdfWaitLockAcquire(connect_list_lock, nullptr);
	InsertTailList(&connect_list, &node->list_entry);
	WdfWaitLockRelease(connect_list_lock);

	return TRUE;
}

conn_item* remove_head() {
	WdfWaitLockAcquire(connect_list_lock, nullptr);
	conn_item* node = NULL;
	if (IsListEmpty(&connect_list)) {
		goto end;
	}
	PLIST_ENTRY item = RemoveHeadList(&connect_list);
	node = CONTAINING_RECORD(item, conn_item, list_entry);
end:
	WdfWaitLockRelease(connect_list_lock);
	return node;
}

void clear_list() {
	WdfWaitLockAcquire(connect_list_lock, nullptr);
	while (!IsListEmpty(&connect_list)) {
		PLIST_ENTRY item = RemoveHeadList(&connect_list);
		conn_item* node = CONTAINING_RECORD(item, conn_item, list_entry);
		ExFreePoolWithTag(node, WDF_DEMO_TAG);
	}
	WdfWaitLockRelease(connect_list_lock);
}