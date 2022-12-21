#pragma once

#include "common/log.h"
#include "rule.h"
#include <ntddk.h>

#define NDIS684
// #define NDIS630
#include <fwpsk.h>
#include <fwpmu.h>
#define INITGUID
#include <guiddef.h>

// Guidgen.exe生成
// {5AEFC185-9374-4AAB-86E3-F86F30D3092A}
DEFINE_GUID(WFP_ESTABLISHED_CALLOUT_V4_GUID, 0x5aefc185, 0x9374, 0x4aab, 0x86, 0xe3, 0xf8, 0x6f, 0x30, 0xd3, 0x9, 0x2a);

// {B81F0D31-1A5D-4632-BA13-E94933AB113F}
DEFINE_GUID(WFP_SUBLAYER_GUID,  0xb81f0d31, 0x1a5d, 0x4632, 0xba, 0x13, 0xe9, 0x49, 0x33, 0xab, 0x11, 0x3f);


#define WFP_SAMPLE_ESTABLISHED_CALLOUT_DISPLAY_NAME	L"WfpEstablishedCalloutName"
#define WFP_SAMPLE_SUB_LAYER_DISPLAY_NAME	L"WfpSampleSubLayerName"
#define WFP_SAMPLE_FILTER_ESTABLISH_DISPLAY_NAME	L"WfpFilterEstablishName"

HANDLE open_engine(); // 打开过滤引擎

void close_engine(); // 关闭过滤引擎

NTSTATUS register_callout(device_object); // 注册callout

void unregister_callout(); // 注销callout

NTSTATUS add_callout(); // 向系统中添加callout对象

void remove_callout();

NTSTATUS add_sublayer(); // 添加子层

void remove_sublayer(); // 移除子层

DWORD add_filter(); // 向系统添加新的筛选器对象 - 通过GUID,关联子层和callout

void remove_filter();

NTSTATUS init_wfp(HANDLE device_object);

void uninit_wfp();