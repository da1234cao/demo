#include "../libdemo/demo_common.h"
#include <Windows.h>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    HANDLE device = CreateFile("\\\\.\\wdf_demo", GENERIC_READ | GENERIC_WRITE,
                             0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);                           
  if(device == INVALID_HANDLE_VALUE) {
    DWORD err_code = GetLastError();
    std::cerr << "fail to open device" << std::endl;
    std::cerr << "error code is: " << err_code << std::endl;
    return -1;
  }

  int ret_len = 0;

  std::string msg = "hello world";
  BOOL result = DeviceIoControl(device, IOCTL_SET_MSG, (LPVOID)(msg.c_str()), msg.size(), NULL, 0, (LPDWORD)&ret_len, NULL);
  if(result == 0) {
    DWORD err_code = GetLastError();
    std::cerr << "fail in DeviceIoControl-IOCTL_SET_MSG. error code is: " << err_code << std::endl;
    return 0;
  } 
  std::string msg_2 = "dlrow olleh";
  result = DeviceIoControl(device, IOCTL_SET_MSG, (LPVOID)(msg_2.c_str()), msg_2.size(), NULL, 0, (LPDWORD)&ret_len, NULL);


  char get_msg[1024] = { 0 };
  result = DeviceIoControl(device, IOCTL_GET_MSG, NULL, 0, &get_msg, 1024, (LPDWORD)&ret_len, NULL);
  std::cout << ret_len << std::endl;
  std::cout << get_msg << std::endl;
  char get_msg_2[1024] = { 0 };
  result = DeviceIoControl(device, IOCTL_GET_MSG, NULL, 0, &get_msg_2, 1024, (LPDWORD)&ret_len, NULL);
  std::cout << ret_len << std::endl;
  std::cout << get_msg_2 << std::endl;


  return 0;
}