#include <stdio.h>

int add_two(int x, int y) { return x + y; };

int main() {
  int x = 5;
  int y = 42;
  int z = add_two(x, y);

  printf("%d\n", z);
  return 0;
}
