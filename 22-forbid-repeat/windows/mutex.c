#include <windows.h>

int main(int argc, char **argv)
{
  HANDLE hmutex = CreateMutex(NULL, FALSE, "FORBID_REPEAT_MUTEXT");
  MessageBox(NULL, TEXT("程序已经启动,禁止重复启动"), TEXT("提示"), MB_OK);
  return 0;
}