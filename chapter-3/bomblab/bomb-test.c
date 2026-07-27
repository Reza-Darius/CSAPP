#include <stdio.h>
#include <stdlib.h>

int func4(int x, int y, int z) {
  // v = rax, eax
  // a = rdx, ecx
  int v, a;

  v = z - y;
  a = !!(v & 1 << 31);

  v += a;
  v >>= 1;
  a = v + y;
  if (a > x) {
    z = a - 1;
    v = func4(x, y, z) * 2;
    return v;
  } else {
    v = 0;
    if (a < x) {
      y = a + 1;
      v = func4(x, y, z) * 2 + 1;
    }
  }
  return v;
}

int func4_2(int x, int y, int z) {
  int mid = y + ((z - y) / 2);
  if (mid > x) {
    return func4_2(x, y, mid - 1) * 2;
  } else if (mid < x){
      return func4_2(x, mid + 1, z) * 2 + 1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int res;

  for (int x = 0; x <= 14; x++) {
    res = func4(x, 0, 14);

    printf("func4() returns %d for %d\n", res, x);
  }


  for (int x = 0; x <= 14; x++) {
    res = func4(x, 0, 14);

    printf("func4_2() returns %d for %d\n", res, x);
  }

  return EXIT_SUCCESS;
}
