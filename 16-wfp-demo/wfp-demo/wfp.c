#include "wfp.h"

HANDLE g_engine_handle = NULL;
UINT32 g_calloutId = 0;
UINT32 g_callout_object_id = 0;
UINT64 g_filter_id = 0;

HANDLE open_engine() {
  // opens a session to the filter engine
  FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, NULL, &g_engine_handle);
  kd_log_debug(("debug: open engine handle is: %p", g_engine_handle));
  return g_engine_handle;
}

void close_engine() {
  // closes a session to a filter engine.
  if (g_engine_handle != NULL) {
    FwpmEngineClose(g_engine_handle);
    g_engine_handle = NULL;
    kd_log_debug(("debug: close engine."));
  }
}

void Wfp_Established_ClassifyFn_V4(
     IN const FWPS_INCOMING_VALUES0* inFixedValues, 
     IN const FWPS_INCOMING_METADATA_VALUES0* inMetaValues, 
     IN OUT void* layerData, 
     IN const FWPS_FILTER0* filter, 
     IN UINT64 flowContext, 
     IN OUT FWPS_CLASSIFY_OUT0* classifyOut) {

  kd_log_debug(("enter in Wfp_Established_ClassifyFn_V4 function."));
  if (!(classifyOut->rights & FWPS_RIGHT_ACTION_WRITE)){
    kd_log_debug(("Wfp_Established_ClassifyFn_V4: classifyOut no FWPS_RIGHT_ACTION_WRITE right."));
    return;
  }
  
  ST_WFP_NETINFO info;
  info.m_uDirect = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_DIRECTION].value.int8;
  info.m_uSrcPort = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_PORT].value.uint16;
  info.m_ulSrcIPAddr = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_ADDRESS].value.uint32;
  info.m_uDstPort = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_PORT].value.uint16;
  info.m_ulDstIPAddr = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_ADDRESS].value.uint32;
  info.m_uProtocalType = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_PROTOCOL].value.uint8;

  classifyOut->actionType = FWP_ACTION_PERMIT;

  if(is_hit_rule(&info)) {
    classifyOut->actionType = FWP_ACTION_BLOCK;
  }

  kd_log_debug(("m_uDirect: %d", info.m_uDirect));
  kd_log_debug(("m_uSrcPort: %d", info.m_uSrcPort));
  kd_log_debug(("m_uDstPort: %lu", info.m_uDstPort));
  kd_log_debug(("m_ulSrcIPAddr: %lu", info.m_ulSrcIPAddr));
  kd_log_debug(("m_ulDstIPAddr: %lu", info.m_ulDstIPAddr));
  kd_log_debug(("m_uProtocalType: %d", info.m_uProtocalType));
  if (classifyOut->actionType == FWP_ACTION_PERMIT) {
    kd_log_debug(("actionType: FWP_ACTION_PERMIT"));
  }
  else if (classifyOut->actionType == FWP_ACTION_BLOCK) {
    kd_log_debug(("actionType: FWP_ACTION_BLOCK"));
  }

  // filter->flags用于指定callout中的操作
  // classifyOut->actionType 指定FWP_ACTION_PERMIT/FWP_ACTION_BLOCK时，清除FWPS_RIGHT_ACTION_WRITE标志
  if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT) {
    classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
  }
}

NTSTATUS Wfp_Established_NotifyFn_V4(
          IN FWPS_CALLOUT_NOTIFY_TYPE notifyType, 
          IN const GUID* filterKey, 
          IN FWPS_FILTER* filter){
  // 添加/删除filter时调用
  kd_log_debug(("enter in Wfp_Established_NotifyFn_V4 function"));
  return STATUS_SUCCESS;
}

void Wfp_Established_FlowDeleteFn_V4(
  IN UINT16 layerId,
  IN UINT32 calloutId,
  IN UINT64 flowContext) {
  kd_log_debug(("enter in Wfp_Established_FlowDeleteFn_V4 function"));
}

NTSTATUS register_callout(HANDLE device_object) {
  // registers a callout with the filter engine
  NTSTATUS status = STATUS_SUCCESS;

  FWPS_CALLOUT callout;
  memset(&callout, 0, sizeof(callout));
  callout.calloutKey = WFP_ESTABLISHED_CALLOUT_V4_GUID;
  callout.flags = 0;
  callout.classifyFn = Wfp_Established_ClassifyFn_V4;
  callout.notifyFn = Wfp_Established_NotifyFn_V4;
  callout.flowDeleteFn = Wfp_Established_FlowDeleteFn_V4;

  status = FwpsCalloutRegister(device_object, &callout, &g_calloutId);
  kd_log_debug(("debug: register callout return: %X", status));
  return status;
}

void unregister_callout() {
  // unregisters a callout from the filter engine.
  FwpsCalloutUnregisterById(g_calloutId);
  g_calloutId = 0;
}

NTSTATUS add_callout() {
  // adds a new callout object to the system.
  FWPM_CALLOUT fwpm_callout;
  memset(&fwpm_callout, 0, sizeof(fwpm_callout));
  fwpm_callout.calloutKey = WFP_ESTABLISHED_CALLOUT_V4_GUID;
  fwpm_callout.displayData.name = WFP_SAMPLE_ESTABLISHED_CALLOUT_DISPLAY_NAME;
  fwpm_callout.displayData.description = WFP_SAMPLE_ESTABLISHED_CALLOUT_DISPLAY_NAME;
  fwpm_callout.flags = 0;
  fwpm_callout.applicableLayer = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4; // 此筛选层允许在建立 TCP 连接或已授权非 TCP 流量时发出通知

  NTSTATUS status = FwpmCalloutAdd(g_engine_handle, &fwpm_callout, NULL, &g_callout_object_id);
  kd_log_debug(("debug: add callout return: %X", status));
  return status;
}

void remove_callout() {
  // removes a callout object from the system
  if (g_engine_handle != NULL && g_callout_object_id != 0) {
    FwpmCalloutDeleteById(g_engine_handle, g_callout_object_id);
    g_callout_object_id = 0;
  }
}

DWORD add_sublayer() {
  //  adds a new sublayer to the system.
  FWPM_SUBLAYER sub_layer;
  memset(&sub_layer, 0, sizeof(sub_layer)); // 一定要初始化，否则FwpmSubLayerAdd调用会报错
  sub_layer.subLayerKey = WFP_SUBLAYER_GUID;
  sub_layer.displayData.description = WFP_SAMPLE_SUB_LAYER_DISPLAY_NAME;
  sub_layer.displayData.name = WFP_SAMPLE_SUB_LAYER_DISPLAY_NAME;
  sub_layer.flags = 0;
  sub_layer.weight = 0xffff;

  DWORD status = FwpmSubLayerAdd(g_engine_handle, &sub_layer, NULL);
  kd_log_debug(("debug: add sublayer return: %lu", status));
  return status;
}

void remove_sublayer() {
  if (g_engine_handle != NULL) {
    FwpmSubLayerDeleteByKey(g_engine_handle, &WFP_SUBLAYER_GUID);
  }
}

NTSTATUS add_filter() {
  // 向系统添加新的筛选器对象
  FWPM_FILTER filter;
  memset(&filter, 0, sizeof(filter));
  filter.displayData.name = WFP_SAMPLE_FILTER_ESTABLISH_DISPLAY_NAME;
  filter.displayData.description = WFP_SAMPLE_FILTER_ESTABLISH_DISPLAY_NAME;
  filter.flags = FWPM_FILTER_FLAG_NONE;
  filter.layerKey = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4; // filter所在层GUID
  filter.subLayerKey = WFP_SUBLAYER_GUID; // 所在子层的GUID
  filter.weight.type = FWP_EMPTY; // filter权重自动分配

  // 匹配条件只有一个。匹配所有的IPv4。以触发callout
  filter.numFilterConditions = 1;
  FWPM_FILTER_CONDITION FilterCondition[1] = { 0 };
  FWP_V4_ADDR_AND_MASK AddrAndMask = { 0 };
  FilterCondition[0].fieldKey = FWPM_CONDITION_IP_LOCAL_ADDRESS; // FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4层只有这一种
  FilterCondition[0].matchType = FWP_MATCH_EQUAL;
  FilterCondition[0].conditionValue.type = FWP_V4_ADDR_MASK;
  FilterCondition[0].conditionValue.v4AddrMask = &AddrAndMask;
  filter.filterCondition = FilterCondition;

  // 调用的callout总是返回FWP_ACTION_BLOCK/FWP_ACTION_PERMIT
  filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
  // filter.action.type = FWP_ACTION_CALLOUT_UNKNOWN;
  filter.action.calloutKey = WFP_ESTABLISHED_CALLOUT_V4_GUID; // 使用的callout

  NTSTATUS nStatus = FALSE;
  nStatus = FwpmFilterAdd(g_engine_handle, &filter, NULL, &g_filter_id);
  kd_log_debug(("debug: add filter return: %X", nStatus));
  return nStatus;
}

void remove_filter()
{
  // 从系统中删除筛选器对象
  if (g_engine_handle != NULL && g_filter_id !=0) {
    FwpmFilterDeleteById(g_engine_handle, g_filter_id);
  }
}

NTSTATUS init_wfp(HANDLE device_object) {
  open_engine(); // 打开过滤引擎
  register_callout(device_object); // 注册callout
  add_callout(); // 向系统中添加callout对象
  add_sublayer(); // 添加子层
  add_filter(); // 向系统添加新的筛选器对象 - 通过GUID,关联子层和callout
  return STATUS_SUCCESS;
}

void uninit_wfp() {
  remove_filter(); // 从系统中删除筛选器对象
  remove_sublayer(); // 移除子层
  remove_callout(); // 从系统中移除callout
  unregister_callout(); // 注销callout
  close_engine(); // 关闭过滤引擎
}