// pp 3.28
// A)
long fun_b (unsigned long x) {
  long val = 0;
  long i;
  for (i = 64; i > 0; i--) {
    // x -> rcx
    // and 1 with x in rcx, upper bits to zero
    int y = 1 & x;
    // add val to rax, val * 2
    val += val;
    // or rcx with rax
    val |= y;
    x >>= 1;
  }
  return val;
}

// B)
// because 64 is hard coded, it will always return true on the first iteration
// C)
// input 5 = 0101
// y = 0001
// val += val = 0
// val |= y = 1
// x = 0010
//
// y = 0
// val += val = 2
// val |= y = 2
// x = 0001
//
//
//y = 1
//val += val = 4
//val |= y = 5
//x = 0
//
// solution (looked up): it creates a mirror image of x bits
