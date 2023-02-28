#include <windows.h>

int main(int argc, char **argv)
{
  // 程序结束后，自动释放该锁;也可以closehandle(hmutex)下
  HANDLE hmutex = CreateMutex(NULL, FALSE, "FORBID_REPEAT_MUTEXT");
  if(hmutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS ) {
    MessageBox(NULL, TEXT("程序已经启动,禁止重复启动"), TEXT("提示"), MB_OK);
    return -1;
  }
  return 0;
}