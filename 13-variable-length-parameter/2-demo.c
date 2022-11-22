#include <stdio.h>
#include <stdarg.h>

void print_log(char * format, ...) {
  va_list va;
  va_start(va, format);
  char buffer[256]; // 小心缓冲区溢出
  vsprintf(buffer, format, va);
  va_end(va);

  printf("%s\n", buffer);
}

int main(int argc, char* argv[])
{
  print_log("variable length parameter:%d-%s", 1, "abc");
  return 0;
}