#pragma once

#include "common/log.h"
#include <ntddk.h>

#define WFP_TAG "wfpt"

#pragma pack(push)
#pragma push(1) // 对齐长度为1

typedef struct {
  USHORT m_uSrcPort;
  USHORT m_uDstPort;
  ULONG  m_ulSrcIPAddr; // 源地址-ipv4
  ULONG  m_ulDstIPAddr; // 目标地址-ipv4
  USHORT m_uProtocalType; 
  USHORT m_uDirect; // 数据包方向。0表示发送，1表示接收
} ST_WFP_NETINFO, * PST_WFP_NETINFO;

typedef struct {
  LIST_ENTRY m_linkPoint;
  ST_WFP_NETINFO m_stWfpNetInfo;
} ST_WFP_NETINFOLIST, *PST_WFP_NETINFOLIST;

#pragma pop()

void init_rule_list();

BOOLEAN add_rule(PVOID buf, ULONG len);

BOOLEAN clear_rules();

BOOLEAN is_hit_rule(PST_WFP_NETINFO buf);

BOOLEAN get_rule_list(PVOID buf, PULONG len);