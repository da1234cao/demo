#include "rule.h"

LIST_ENTRY g_rule_list;
KSPIN_LOCK spin_lock;

void init_rule_list()
{
  InitializeListHead(&g_rule_list);
  KeInitializeSpinLock(&spin_lock);
}

BOOLEAN add_rule(PVOID buf, ULONG len)
{
  BOOLEAN succ = FALSE;
  if (len < sizeof(ST_WFP_NETINFO)) {
    kd_log_debug(("add_rule: incomplete data.input size is %u.need size is %u",len, sizeof(ST_WFP_NETINFOLIST)));
    return succ;
  }
  PST_WFP_NETINFO rule_info = (PST_WFP_NETINFO)buf;

  PST_WFP_NETINFOLIST rule_node = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(ST_WFP_NETINFOLIST), WFP_TAG);
  if (rule_node == NULL) {
    kd_log_debug(("add_rule: fail in ExAllocatePool2"));
    return succ;
  }
  memset(rule_node, 0, sizeof(ST_WFP_NETINFOLIST));
  rule_node->m_stWfpNetInfo.m_uSrcPort = rule_info->m_uSrcPort;
  rule_node->m_stWfpNetInfo.m_uDstPort = rule_info->m_uDstPort;
  rule_node->m_stWfpNetInfo.m_ulSrcIPAddr = rule_info->m_ulSrcIPAddr;
  rule_node->m_stWfpNetInfo.m_ulDstIPAddr = rule_info->m_ulDstIPAddr;
  rule_node->m_stWfpNetInfo.m_uProtocalType = rule_info->m_uProtocalType;
  rule_node->m_stWfpNetInfo.m_uDirect = rule_info->m_uDirect;

  KIRQL	OldIRQL;
  KeAcquireSpinLock(&spin_lock, &OldIRQL);
  InsertTailList(&g_rule_list, &rule_node->m_linkPoint);
  KeReleaseSpinLock(&spin_lock, OldIRQL);

  kd_log_debug(("insert one entry"));
  succ = TRUE;
  return succ;
}

BOOLEAN clear_rules()
{ 
  BOOLEAN succ = FALSE;

  KIRQL	OldIRQL;
  KeAcquireSpinLock(&spin_lock, &OldIRQL);
  while (IsListEmpty(&g_rule_list) == FALSE) {
    PLIST_ENTRY info = RemoveTailList(&g_rule_list);
    PST_WFP_NETINFOLIST rule = CONTAINING_RECORD(info, ST_WFP_NETINFOLIST, m_linkPoint);
    ExFreePoolWithTag(rule, WFP_TAG);
  }
  KeReleaseSpinLock(&spin_lock, OldIRQL);
  succ = TRUE;
  return succ;
}

BOOLEAN is_hit_rule(PST_WFP_NETINFO buf)
{
  BOOLEAN is_hit = FALSE;

  KIRQL	OldIRQL;
  KeAcquireSpinLock(&spin_lock, &OldIRQL);
  if (IsListEmpty(&g_rule_list)) {
    kd_log_debug(("is hit rule: rule list is empty"));
    goto end;
  }
  PLIST_ENTRY enter = g_rule_list.Flink;
  while (enter != &g_rule_list) {
    PST_WFP_NETINFOLIST rule = CONTAINING_RECORD(enter, ST_WFP_NETINFOLIST, m_linkPoint);
    // 只比较端口
    if (rule->m_stWfpNetInfo.m_uDstPort == buf->m_uDstPort) {
      is_hit = TRUE;
      goto end;
    }
    enter = enter->Flink;
  }
  
end:
  KeReleaseSpinLock(&spin_lock, OldIRQL);
  return is_hit;
}

BOOLEAN get_rule_list(PVOID buf, PULONG len)
{
  BOOLEAN succ = TRUE;

  KIRQL	OldIRQL;
  KeAcquireSpinLock(&spin_lock, &OldIRQL);
  if (IsListEmpty(&g_rule_list) == TRUE) {
    *len = 0;
    kd_log_debug(("rule list is empty"));
    goto end;
  }

  PLIST_ENTRY enter = g_rule_list.Flink;
  ULONG size = 0; // 获取规则存储空间的大小
  while (enter != &g_rule_list) {
    size += sizeof(ST_WFP_NETINFO);
    enter = enter->Flink;
    kd_log_debug(("one entry"));
  }
  if (size > *len) {
    *len = size;
    succ = FALSE; 
    kd_log_debug(("debug: %lu size is not enough, need %lu.", *len, size));
    goto end; // 获取失败，返回需要的空间
  }
  else {
    *len = size;
    enter = g_rule_list.Flink;
    char* buf_now = buf;
    while (enter != &g_rule_list) {
      PST_WFP_NETINFOLIST rule = CONTAINING_RECORD(enter, ST_WFP_NETINFOLIST, m_linkPoint);
      memcpy(buf_now, &rule->m_stWfpNetInfo, sizeof(ST_WFP_NETINFO));
      buf_now += sizeof(ST_WFP_NETINFO);
      enter = enter->Flink;
    }
    kd_log_debug(("debug: get_rule_list rule string len: %lu", size));
  }

end:
  KeReleaseSpinLock(&spin_lock, OldIRQL);
  return succ;
}


