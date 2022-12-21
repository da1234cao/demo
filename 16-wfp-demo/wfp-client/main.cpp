#include "main.h"

int main(int argc, char* argv[]) {
  HANDLE device = CreateFile(DRIVER_OBJECT_SYM_NAME, GENERIC_READ | GENERIC_WRITE,
                             0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);                           
  if(device == INVALID_HANDLE_VALUE) {
    std::cerr << "fail to open device" << std::endl;
    return -1;
  }
  
  ST_WFP_NETINFO baidu_info = {0};
  baidu_info.m_uDstPort = 443;

  // 发送规则到内核驱动
  unsigned long ret_len = 0;
  BOOL result = DeviceIoControl(device, ADD_RULE, (LPVOID)&baidu_info, sizeof(baidu_info), NULL, 0, (LPDWORD)&ret_len, NULL);
  if(result == FALSE) {
    std::cerr << "fail in DeviceIoControl: ADD_RULE" << std::endl;
    return -1;
  }

  // 读取内核驱动中的规则
  char read_buff[2048] = {0};
  ret_len = 2048;
  result = DeviceIoControl(device, GET_RULE_LIST, NULL, 0, read_buff, 2048, (LPDWORD)&ret_len, NULL);
  if(result == FALSE) {
    std::cerr << "fail in DeviceIoControl: GET_RULE_LIST. Maybe the read_buff is not big enough" << std::endl;
    DWORD err_code = GetLastError();
    std::cerr << "err code: " << err_code << std::endl;
    return -1;
  }

  int size = ret_len / sizeof(ST_WFP_NETINFO);
  PST_WFP_NETINFO buf = (PST_WFP_NETINFO)read_buff;
  std::cout << "rule list size: " << size << std::endl;
  for(int i = 0; i < size; i++) {
    std::cout << "m_uSrcPort: "     << buf[i].m_uSrcPort << std::endl
              << "m_uDstPort: "       << buf[i].m_uDstPort << std::endl
              << "m_ulSrcIPAddr: "    << buf[i].m_ulSrcIPAddr << std::endl
              << "m_ulDstIPAddr: "    << buf[i].m_ulDstIPAddr << std::endl
              << "m_uProtocalType: "  << buf[i].m_uProtocalType << std::endl
              << "m_uDirect: "        << buf[i].m_uDirect << std::endl;
  }
  CloseHandle(device);
  return 0;
}