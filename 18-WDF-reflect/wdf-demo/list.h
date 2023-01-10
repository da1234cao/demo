#pragma once
#include <ntddk.h>
#include <wdf.h>	

#define ENTER_MAX_SIZE 1024 // 每个节点的最大大小为1024字节
#define WDF_DEMO_TAG 'wdft'

extern LIST_ENTRY connect_list;
extern WDFWAITLOCK connect_list_lock;

typedef struct {
	LIST_ENTRY list_entry;
	char connect[ENTER_MAX_SIZE];
}conn_item;

void init_list();

BOOLEAN add_tail(PVOID inbuf, size_t inlen);

conn_item* remove_head();

void clear_list();
