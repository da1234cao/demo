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

// Guidgen.exe����
// {5AEFC185-9374-4AAB-86E3-F86F30D3092A}
DEFINE_GUID(WFP_ESTABLISHED_CALLOUT_V4_GUID, 0x5aefc185, 0x9374, 0x4aab, 0x86, 0xe3, 0xf8, 0x6f, 0x30, 0xd3, 0x9, 0x2a);

// {B81F0D31-1A5D-4632-BA13-E94933AB113F}
DEFINE_GUID(WFP_SUBLAYER_GUID,  0xb81f0d31, 0x1a5d, 0x4632, 0xba, 0x13, 0xe9, 0x49, 0x33, 0xab, 0x11, 0x3f);


#define WFP_SAMPLE_ESTABLISHED_CALLOUT_DISPLAY_NAME	L"WfpEstablishedCalloutName"
#define WFP_SAMPLE_SUB_LAYER_DISPLAY_NAME	L"WfpSampleSubLayerName"
#define WFP_SAMPLE_FILTER_ESTABLISH_DISPLAY_NAME	L"WfpFilterEstablishName"

HANDLE open_engine(); // �򿪹�������

void close_engine(); // �رչ�������

NTSTATUS register_callout(device_object); // ע��callout

void unregister_callout(); // ע��callout

NTSTATUS add_callout(); // ��ϵͳ�����callout����

void remove_callout();

NTSTATUS add_sublayer(); // ����Ӳ�

void remove_sublayer(); // �Ƴ��Ӳ�

DWORD add_filter(); // ��ϵͳ����µ�ɸѡ������ - ͨ��GUID,�����Ӳ��callout

void remove_filter();

NTSTATUS init_wfp(HANDLE device_object);

void uninit_wfp();