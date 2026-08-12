/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include "cachelab.h"
#include <stdio.h>

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans32(int M, int N, int A[N][M], int B[M][N]);
void trans64(int M, int N, int A[N][M], int B[M][N]);
void trans61(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
  if (M == 32 && N == 32) {
    trans32(M, N, A, B);
  }
  if (M == 64 && N == 64) {
    trans64(M, N, A, B);
  } else {
    trans61(M, N, A, B);
  }
  return;
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans block wise transposition with 8 values loaded at a time
 */
char trans_desc32[] = "32 wide row block transposition for 32 width";
void trans32(int M, int N, int A[N][M], int B[M][N]) {
  int i, x, y;

  int n0, n1, n2, n3, n4, n5, n6, n7;

  for (y = 0; y < N; y += 8) {
    for (x = 0; x < N; x += 8) {
      // we go top to bottom
      for (i = 0; i < 8; i++) {
        n0 = A[i + y][0 + x];
        n1 = A[i + y][1 + x];
        n2 = A[i + y][2 + x];
        n3 = A[i + y][3 + x];
        n4 = A[i + y][4 + x];
        n5 = A[i + y][5 + x];
        n6 = A[i + y][6 + x];
        n7 = A[i + y][7 + x];

        B[0 + x][i + y] = n0;
        B[1 + x][i + y] = n1;
        B[2 + x][i + y] = n2;
        B[3 + x][i + y] = n3;
        B[4 + x][i + y] = n4;
        B[5 + x][i + y] = n5;
        B[6 + x][i + y] = n6;
        B[7 + x][i + y] = n7;
      }
    }
  }
}

/*
 * trans block wise transposition with 8 values loaded at a time
 */
char trans_desc64[] = "4 byte wide row block transposition for 64 width";
void trans64(int M, int N, int A[N][M], int B[M][N]) {
  int i, x, y;

  int n0, n1, n2, n3, n4, n5, n6, n7;

  // we assume N == M
  for (y = 0; y < N; y += 8) {
    for (x = 0; x < N; x += 8) {
      // write elements into temporary location in B as to not evict them
      for (i = 0; i < 4; i++) {
        n0 = A[i + y][0 + x];
        n1 = A[i + y][1 + x];
        n2 = A[i + y][2 + x];
        n3 = A[i + y][3 + x];
        n4 = A[i + y][4 + x];
        n5 = A[i + y][5 + x];
        n6 = A[i + y][6 + x];
        n7 = A[i + y][7 + x];

        B[0 + x][i + y] = n0;
        B[1 + x][i + y] = n1;
        B[2 + x][i + y] = n2;
        B[3 + x][i + y] = n3;
        B[0 + x][i + y + 4] = n4;
        B[1 + x][i + y + 4] = n5;
        B[2 + x][i + y + 4] = n6;
        B[3 + x][i + y + 4] = n7;
      }
      // move temporaries into the correct position
      for (int h = 0; h < 4; h++) {
        // get the temporary elements from previous loop
        // row wise iteratiotn
        n0 = B[x + h][0 + y + 4];
        n1 = B[x + h][1 + y + 4];
        n2 = B[x + h][2 + y + 4];
        n3 = B[x + h][3 + y + 4];

        // read new values in column wise iteration
        n4 = A[4 + y][h + x];
        n5 = A[5 + y][h + x];
        n6 = A[6 + y][h + x];
        n7 = A[7 + y][h + x];

        // write new values into old temporary location since they are still
        // cached
        B[x + h][0 + y + 4] = n4;
        B[x + h][1 + y + 4] = n5;
        B[x + h][2 + y + 4] = n6;
        B[x + h][3 + y + 4] = n7;

        // old temporaries into new cache lines, evicting the old one
        // but thats okay because they are already done
        B[4 + x + h][0 + y] = n0;
        B[4 + x + h][1 + y] = n1;
        B[4 + x + h][2 + y] = n2;
        B[4 + x + h][3 + y] = n3;

        // swap the remaining elements, the last sub block down right
        B[4 + x + h][4 + y] = A[4 + y][4 + x + h];
        B[4 + x + h][5 + y] = A[5 + y][4 + x + h];
        B[4 + x + h][6 + y] = A[6 + y][4 + x + h];
        B[4 + x + h][7 + y] = A[7 + y][4 + x + h];
      }
    }
  }
}

void trans61(int M, int N, int A[N][M], int B[M][N]) {
  // N = 61, M = 67
  int y, x, i, j;
  int block_size = 17;

  for (y = 0; y < N; y += block_size) {
    for (x = 0; x < M; x += block_size) {
      for (i = y; i < y + block_size && i < N; i++) {
        for (j = x; j < x + block_size && j < M; j++) {
          B[j][i] = A[i][j];
        }
      }
    }
  }

  return;
};

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
  /* Register your solution function */
  registerTransFunction(transpose_submit, transpose_submit_desc);

  /* Register any additional transpose functions */
  registerTransFunction(trans32, trans_desc32);
  registerTransFunction(trans64, trans_desc64);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
  int i, j;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; ++j) {
      if (A[i][j] != B[j][i]) {
        return 0;
      }
    }
  }
  return 1;
}
