#include <stdio.h>
/*
 * 2.58 ◆◆
 * Write a procedure is_little_endian that will return 1 when compiled and run
 on a little-endian machine, and will return 0 when compiled and run on a bigendian
 machine. This program should run on any machine, regardless of its word
 size.
 */
int is_little_endian() {
    int x = 1 << 31;
    char* ptr = (char*) &x;
    return ptr[0] == 0;
};

/*
 * 2.59 ◆◆
 * Write aCexpression that will yield a word consisting of the least significant byte of
 x and the remaining bytes of y. For operands x=0x89ABCDEF and y=0x76543210,
 this would give 0x765432EF.
 */
int two_five_nine(int x, int y) {
    char* x_ptr = (char*) &x;
    char* y_ptr = (char*) &y;
    y_ptr[0] = x_ptr[0];
    return *(int*) y_ptr;
};

/*
 * 2.60 ◆◆
 Suppose we number the bytes in a w-bit word from 0 (least significant) to w/8 − 1
 (most significant). Write code for the following C function, which will return an
 unsigned value in which byte i of argument x has been replaced by byte b:
 unsigned replace_byte (unsigned x, int i, unsigned char b);
 Here are some examples showing how the function should work:
 replace_byte(0x12345678, 2, 0xAB) --> 0x12AB5678
 replace_byte(0x12345678, 0, 0xAB) --> 0x123456AB
 */
unsigned replace_byte (unsigned x, int i, unsigned char b){
    char* x_ptr = (char*) &x;
    int idx = 3 - i;
    x_ptr[i] = b;
    return x;
};

/*
  RULES

 . Assumptions
 Integers are represented in two’s-complement form.
 Right shifts of signed data are performed arithmetically.
 Data type int is w bits long. For some of the problems, you will be given a
 specific value for w, but otherwise your code should work as long as w is a
 multiple of 8. You can use the expression sizeof(int)<<3 to compute w.

 . Forbidden
 Conditionals (if or ?:), loops, switch statements, function calls, and macro
 invocations.
 Division, modulus, and multiplication.
 Relative comparison operators (<, >, <=, and >=).

 . Allowed operations
 All bit-level and logic operations.
 Left and right shifts, but only with shift amounts between 0 and w − 1.
 Addition and subtraction.
 Equality (==) and inequality (!=) tests. (Some of the problems do not allow
 these.)
 Integer constants INT_MIN and INT_MAX.
 Casting between data types int and unsigned, either explicitly or implicitly.
 */

/*
 * 2.62 ◆◆◆
 Write a function int_shifts_are_arithmetic() that yields 1 when run on a
 machine that uses arithmetic right shifts for data type int and yields 0 otherwise.
 Your code should work on a machine with any word size. Test your code on several
 machines.

 mask = 0 - 1 = 1111
 y = mask >> 1 = 1111
 mask ^ y = 0000
 !(mask ^ y) = 1


 y = mask >> 1 = 0111
 mask ^ y = 1000
 !(1000) = 0

 */
int int_shifts_are_arithmetic() {
    int x = 0 - 1;
    return !((x >> 1) ^ x);
};

/*
 * 2.63 ◆◆◆
 Fill in code for the following C functions. Function srl performs a logical right
 shift using an arithmetic right shift (given by value xsra), followed by other operations
 not including right shifts or division. Function sra performs an arithmetic
 right shift using a logical right shift (given by value xsrl), followed by other
 operations not including right shifts or division. You may use the computation
 8*sizeof(int) to determine w, the number of bits in data type int. The shift
 amount k can range from 0 to w − 1.
 */
/*

// 0 case
x = 1101  k = 0
xsra = 1101
output = 1101

n = w_size - k = 4
check = ~!(w_size ^ n)
0100 ^ 0100 = 0 = 1 = 0
check << 4
check - 1 = 1111


// common case
x = 1100, k = 1
output = 0110
xsra = 1110
check = 0100 ^ 0011 = !0111 = ~0 = 1
n = w_size - k = 3
check << 3 = 1000
1000 - 1 = 0111


// w - 1 case
x = 1000 = 8
k = 3
output = 0001
n = w_size - k = 1
1 << 1 = 0010
0010 - 1 = 0001

 */
unsigned srl(unsigned x, int k) {
    /* Perform shift arithmetically */
    unsigned xsra = (int) x >> k;
    int w_size = 8 * sizeof(int);

    // creating mask of bits we want to keep
    int n = w_size - k;
    int check = ~!(w_size - n);
    int mask = (check << n) - 1;

    return xsra & mask;
};

/*

// common case
x = 1101, k = 2
xsrl = 0011
expected output = 1111

// k = 0 check
n = w_size - k = 2
check = !(w_size - n)
check = w_size - n = 2 = 0 = 1
mask = (check << 2) - 1 = 0011

sign = (1 << w_size) & x
0 = positive, 1 = negative

// negative case
sign_mask = 0 - sign = 1111
~mask = 1100
sign_mask = 1111 & ~mask = 1100

xsrl | sign_mask


// common case positive
x = 0110, k = 1
xsrl = 0011
expected output = 0011

n = 4 - 1 = 3
check = 4 - 3 = 1 = 0 = 1
mask = 1 << 3 - 1 = 1000 - 1 = 0111
sign = 0
sign_mask = 0 - sign = 0
sign_mask = 0000 & ~mask = 0000 & 1000 = 0000
xsrl | sign_mask = 0011


// k = 0 negative case
x = 1000, k = 0
xsrl = 1000
expected_output = 1000

n = 4 - 0 = 4
check = 4 - n = 0 = 1 = 0
mask = 0 << 4 - 1 = 1111

sign = 1
sign_mask = 0 - 1 = 1111
sign_mask & ~mask = 1111 & 0000 = 0000
xsrl | sign_mask = 1000 | 0000 = 1000

!0 = 1 = 0
!1 = 0 = 1

 */
int sra(int x, int k) {
    /* Perform shift logically */
    int xsrl = (unsigned) x >> k;
    int w_size = 8 * sizeof(int);

    // creating mask of bits we want to keep
    int n = w_size - k;
    int check = ~!(w_size - n);
    int mask = (check << n) - 1;

    int sign = (1 << (w_size - 1)) & x;
    int sign_mask = ~!sign;
    sign_mask = sign_mask & ~mask;

    return xsrl | sign_mask;
};

/*
2.64 ◆
Write code to implement the following function:
    Return 1 when any odd bit of x equals 1; 0 otherwise.
Assume w=32
Your function should follow the bit-level integer coding rules (page 164),
except that you may assume that data type int has w = 32 bits.
1010
 */
int any_odd_one(unsigned x) {
    int mask = 0xAAAAAAAA;
    return !!(x & mask);
};

/*
2.65 ◆◆◆◆
Write code to implement the following function:

Return 1 when x contains an odd number of 1s; 0 otherwise.
Assume w=32
int odd_ones(unsigned x);

Your function should follow the bit-level integer coding rules (page 164),
except that you may assume that data type int has w = 32 bits.
Your code should contain a total of at most 12 arithmetic, bitwise, and logical
operations.

x = 0100 = 4
output = 1
x = 1000 = 8
output = 1
x = 0010 = 2
output = 1
x = 1101 = 13

x = 0011 = 3
output = 0
x = 0110 = 6
x = 1100 = 12
output = 0
111100 = 60
*/
int odd_ones(unsigned x){
    int mask = 0xAAAAAAAA;
    return 0;
};

int main() {
    printf("is little endian: %d\n", is_little_endian());
    printf("2.59: %x\n", two_five_nine(0x89ABCDEF , 0x76543210));
    printf("replace byte: %x\n", replace_byte(0x12345678, 2, 0xAB));
    printf("shifts are arithmetic: %d\n",int_shifts_are_arithmetic());
    return 0;
}
