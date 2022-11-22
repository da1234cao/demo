#include <stdio.h>
#include <stdarg.h>

int max_int(int n, ...) {
  va_list ap;
  int i, current, largest;

  va_start(ap, n);
  largest = va_arg(ap, int);

  for (i = 0; i < n; i++) {
    current = va_arg(ap, int);
    if (current > largest)
    largest = current;
  }
  
  va_end(ap);
  return largest;
}

int main(int argc, char* argv[])
{
  int max_num = max_int(3, 10, 30, 20);
  printf("%d\n", max_num);
}