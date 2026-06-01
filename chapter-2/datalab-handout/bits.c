/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:

  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code
  must conform to the following style:

  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>

  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.


  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 *
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce
 *      the correct answers.
 */

#endif
// 1
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  int m = x & y;
  int m2 = ~x & ~y;

  return ~m & ~m2;
}
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) { return 1 << 31; }
// 2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  // x + 1 == ~x is only true for TMax and -1
  // so tmax + 1 ^ ~x = 0

  // we have to eliminate the x = -1 case
  int offset = !(x ^ ~0);

  return !((x + 1 + offset) ^ (~x));
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  // we build up the mask of 1010 1010 bytes
  int m, r, y;
  m = 0xAA;
  m += m << 8;
  m += m << 16;
  r = x & m;
  y = !(r ^ m);
  return y;
}
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) { return ~x + 1; }
// 3
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0'
 * to '9') Example: isAsciiDigit(0x35) = 1. isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int lower = x + (~0x30 + 1);
  int low_ok = !((lower >> 31) & 1);

  int upper = 0x39 + (~x + 1);
  int upper_ok = !((upper >> 31) & 1);

  return low_ok & upper_ok;
}
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  // results 1 for "true" and 0 for "false"
  int cond = !!x;
  // create a 1111 mask for true, and 0000 mask for false
  int mask = (cond << 31) >> 31;
  int arm1 = y & mask;
  int arm2 = z & ~mask;
  return arm1 | arm2;
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int diff = y + (~x + 1);
  int diff_sign = (diff >> 31) & 1;
  int x_sign = (x >> 31) & 1;
  int y_sign = (y >> 31) & 1;
  int sign_diff = x_sign ^ y_sign;

  return (sign_diff & x_sign) | (!sign_diff & !diff_sign);
}
// 4
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int logicalNeg(int x) {
  int y = ((x | (~x + 1)) >> 31) & 1;
  return y ^ 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4


 -5 = 1111 1111 1011 ^ = 0000 0000 0100
 TMIN = 1000 0000 0000 = 0111 1111 1111

// normal case, x = 13 + 64 = 77
expected output = 8

int s = !(x >> 31);
s = 1 if x is positive, which we add later

01001101
y = !!(x >> 4)
00000100
y = 1
bits = y << 2 = 4
c += bits
x >>= bits

00000100
y = !!(x >> 2)
00000001
y = 1
bits = y << 1 = 2
c += bits = 6
x >>= bits

00000001
y = !!(x)
y = 1
bits += y = 7

return bits + sign

 */
int howManyBits(int x) {
  int y, bits, count;
  // normalize number so a negative number can be counted like a positive
  x = x ^ (x >> 31);

  count = 0;

  y = !!(x >> 16); // 1 if one of the top 16 bits is set
  bits = y << 4;   // a set bit means we need 16 bits at least
  count += bits;   // add them to our counter
  x >>= bits;      // shift the whole number

  y = !!(x >> 8);
  bits = y << 3;
  count += bits;
  x >>= bits;

  y = !!(x >> 4);
  bits = y << 2;
  count += bits;
  x >>= bits;

  y = !!(x >> 2);
  bits = y << 1;
  count += bits;
  x >>= bits;

  y = !!(x >> 1);
  bits = y;
  count += bits;
  x >>= bits;

  y = !!(x);
  bits = y;
  count += bits;

  return count + 1;
}
// float
/*
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  int n, e_mask, m_mask, sign, e, m;

  // normalizing sign bit to 0
  n = uf & ~(1 << 31);

  if (!n) {
    // uf is 0, we can just return
    return uf;
  }

  e_mask = 0xFF << 23;
  m_mask = ~(0x1FF << 23);

  sign = uf & (1 << 31);
  e = ((n & e_mask) >> 23);
  m = n & m_mask;

  // return if uf is NaN
  if (!(e ^ 0xFF)) {
    return uf;
  };

  if (e) {
    // normalized case

    // adding 1 to exponent field multiplies by 2
    e += 1;
    e <<= 23;

    return sign + e + m;
  } else {
    // denormalized case, we can just shift the mantissa
    return sign + (m << 1);
  }
}
/*
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  unsigned invalid;
  int n, e_mask, m_mask, sign, e, E, m, res, idx, k;

  invalid = 0x80000000;

  // normalizing sign bit to 0
  n = uf & ~(1 << 31);

  if (!n) {
    // uf is 0, we can just return 0
    return 0;
  }

  e_mask = 0xFF << 23;
  m_mask = ~(0x1FF << 23);

  sign = uf & (1 << 31);
  e = ((n & e_mask) >> 23);
  E = e - 127;
  m = n & m_mask;

  // NaN and Inf
  if (!(e ^ 0xFF)) {
    return invalid;
  };

  // if E is negative, we can return 0 immediately because the number is smaller
  // than 1
  if (E < 0) {
    return 0;
  };

  res = 0;
  idx = 0;

  // we add the leading 1
  m += 1 << 23;

  while (idx < 24) {
    // if the bit is set
    if (m & 1 << (23 - idx)) {
      // shift amount: E - ith position in the mantissa
      k = E - idx;

      if (k > 31) {
        return invalid;
      }

      // negative shift means we have a fractured value
      if (k < 0) {
        break;
      }

      res += 1 << k;
    }
    idx += 1;
  }

  if (sign) {
    res = ~res + 1;
  }

  return res;
}
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatPower2(int x) { return 2; }
