#include "main.h"

int main(int argc, char* argv[]) {
  HANDLE device = CreateFile(DRIVER_OBJECT_SYM_NAME, GENERIC_READ | GENERIC_WRITE,
                             0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);                           
  if(device == INVALID_HANDLE_VALUE) {
    std::cerr << "fail to open device" << std::endl;
    return -1;
  }
  
  std::string msg = "hello,world";
  unsigned long ret_len = 0;
  BOOL result = DeviceIoControl(device, IOCTL_OBJECT_TEST_REFLECT, (LPVOID)msg.c_str(), msg.size(), NULL, 0, (LPDWORD)&ret_len, NULL);
  if(result == FALSE) {
    std::cerr << "fail in DeviceIoControl: IOCTL_OBJECT_TEST_REFLECT" << std::endl;
    return -1;
  }

  char read_buff[1024] = {0};
  ret_len = 1024;
  result = DeviceIoControl(device, IOCTL_OBJECT_TEST_READ, NULL, 0, read_buff, 1024, (LPDWORD)&ret_len, NULL);
  if(result == FALSE) {
    std::cerr << "fail in DeviceIoControl: IOCTL_OBJECT_TEST_READ" << std::endl;
    DWORD err_code = GetLastError();
    std::cerr << "err code: " << err_code << std::endl;
    return -1;
  }

  std::cout << "get content:" << std::string(read_buff) << std::endl;
  std::cout << "length: " << ret_len << std::endl;

  CloseHandle(device);
  return 0;
}