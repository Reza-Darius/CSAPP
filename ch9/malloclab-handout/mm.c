/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define IS_ALIGNED(bp) ((((size_t)bp) & ~(0x7)) == 0 ? 1 : 0)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4             /* Word and header/footer size (bytes) */
#define DSIZE 8             /* Double word size (bytes) */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, prv_free, alloc) ((size) | (alloc) | (prv_free << 1))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
/* Sets the pointed at hdr size field */
#define SET_SIZE(p, size) (PUT(p, ((GET(p) & 0x7) | ALIGN(size))))
/* Allocaton is signaled by the first bit */
#define GET_ALLOC(p) (GET(p) & 0x1)
/* Sets the pointed at hdr alloc field */
#define SET_ALLOC(p, alloc) (PUT(p, ((GET(p) & ~0x1) | alloc)))
// the second bit marks whether the previous page is deallocted
#define GET_PREV_FREE(p) ((GET(p) & 0x2) > 0 ? 1 : 0)
/* Sets the pointed at hdr prev_free field */
#define SET_PREV_FREE(p, prev_free)                                            \
  (PUT(p, ((GET(p) & ~0x2) | (prev_free << 1))))

#define GET_NEXT_FREE(bp) ((void *)GET((char *)(bp) + WSIZE))
#define SET_NEXT_FREE(bp, np) (PUT(((char *)(bp) + WSIZE), ((unsigned int)np)))

#define GET_PREV(bp) ((void *)GET((char *)(bp)))
#define SET_PREV(bp, pp) (PUT(((char *)(bp)), ((unsigned int)np)))

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Block size needed for a request, the request size plus size for hdr and
 * footer rounded to alignment*/
#define BLOCK_SIZE(req_size) (ALIGN(req_size + (WSIZE * 2)))
/* Checks if the block is the epilogue block */
#define IS_END(bp) (GET_SIZE(HDRP(bp)) == 0 && GET_ALLOC(HDRP(bp)) == 1 ? 1 : 0)

char *list_head;
char *heap_end;

void *find_free_block(void *bp, size_t block_size);
void *grow_heap(size_t size);
void *coalesce(void *bp);
void *split(void *bp, size_t block_size);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
  // get a start allocation
  if (mem_sbrk(WSIZE * 4) < 0) {
    return -1;
  }

  list_head = mem_heap_lo();
  // end points to one over the max heap
  heap_end = list_head + WSIZE * 4;

  // we set the start block to size 8 and allocated
  PUT(list_head + WSIZE, PACK(DSIZE, 0, 1));

  // end block has special sentinal value of allocated without a size
  PUT(list_head + (WSIZE * 3), PACK(0, 0, 1));

  // point to epiloge block
  list_head += (WSIZE * 4);

  assert(!IS_ALIGNED(list_head));
  return 0;
}

// grows the heap by size bytes, returns block pointer to new free block
void *grow_heap(size_t size) {
  void *new;
  int prev_free;

  // round up to multiples of 8 and make space for epilogue header
  size = ALIGN(size);

  if ((new = mem_sbrk(size)) < 0) {
    return NULL;
  }

  // sanity check
  assert(new == heap_end);

  heap_end = new + size;

  assert(IS_END(new));

  // new free block, with previous "prev free" flag carried over
  prev_free = GET_PREV_FREE(HDRP(new));
  PUT(HDRP(new), PACK(size, prev_free, 0));
  PUT(FTRP(new), PACK(size, prev_free, 0));

  // new epilogue, with previous block marked as free
  PUT(HDRP(heap_end), PACK(0, 1, 1));

  return coalesce(new);
}

// coalesces next and previous block if appropiate, takes a block pointer
//
// returns the pointer to the coalesced block
void *coalesce(void *bp) {
  void *nxt_bl, *prev_bl;
  size_t new_size, prev_free, next_free;

  nxt_bl = NEXT_BLKP(bp);

  next_free = !GET_ALLOC(HDRP(nxt_bl));
  prev_free = GET_PREV_FREE(HDRP(bp));

  // coalesce with next block if possible
  if (next_free) {
    #ifdef DEBUG
    printf("coalesced with next\n");
    #endif
    // add sizes together, and write new values
    new_size = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(nxt_bl));
    PUT(HDRP(bp), PACK(new_size, prev_free, 0));
    PUT(FTRP(bp), PACK(new_size, prev_free, 0));
  }

  // coalesce with previous block if possible
  if (prev_free) {
    #ifdef DEBUG
    printf("coalesced with prev\n");
    #endif
    prev_bl = PREV_BLKP(bp);

    // invariant: we cant ever have two free blocks adjacent
    // so we assert that the the block before the previous block wasnt declared
    // unallocated
    assert(!GET_PREV_FREE(HDRP(prev_bl)));

    // add sizes together, and write new values
    new_size = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(prev_bl));
    PUT(HDRP(prev_bl), PACK(new_size, 0, 0));
    PUT(FTRP(prev_bl), PACK(new_size, 0, 0));

    // update the return pointer
    bp = prev_bl;
  }
  return bp;
}

// walk the list and find a "first fit" block
//
// returns null if the heap is full or it cant find an appropiate block
void *find_free_block(void *bp, size_t req_size) {
  assert(req_size % 8 == 0);

  while (!IS_END(bp) && (char *)bp < heap_end) {
    if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= req_size) {
      #ifdef DEBUG
      printf("found block at: %p for req_size: %zu\n", bp, req_size);
      #endif
      return bp;
    }
    bp = NEXT_BLKP(bp);
  }
  return NULL;
}

/// splits off split_n from block bp
///
/// returns pointer to split off block, or NULL on error
void *split(void *bp, size_t split_n) {
  assert(split_n % 8 == 0);
  void *split_block;
  size_t old_size = GET_SIZE(HDRP(bp));

  // we cant split the prologue block
  if (IS_END(bp)) {
    return NULL;
  }

  // we cant split off more than the block has
  if (old_size < split_n) {
    return NULL;
  };

  if (old_size == split_n) {
    return bp;
  }

  if ((old_size - split_n) < 2 * WSIZE) {
    return NULL;
  };

  // update old header
  SET_SIZE(HDRP(bp), old_size - split_n);

  // in case we are splitting a free block
  if (!GET_ALLOC(HDRP(bp))) {
    PUT(FTRP(bp), GET(HDRP(bp)));
  }

  // calculate offset, and write new split off block
  split_block = (char *)bp + (old_size - split_n);
  PUT(HDRP(split_block), PACK(split_n, 0, 0));
  PUT(FTRP(split_block), PACK(split_n, 0, 0));

  return split_block;
};

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t req_size) {
  #ifdef DEBUG
  printf("malloc called: %zu\n", req_size);
  #endif //DEBUG
  void *block;
  size_t block_size;

  if (req_size == 0) {
    return NULL;
  }

  if (req_size < 2 * DSIZE) {
    req_size = 2 * DSIZE;
  }

  block_size = BLOCK_SIZE(req_size);

  if ((block = find_free_block(list_head, block_size)) == NULL) {
    // we need to extend the heap
    if ((block = grow_heap(block_size)) == NULL) {
      printf("grow heap error");
      return NULL;
    }
  }

  if (GET_SIZE(HDRP(block)) > block_size) {
    #ifdef DEBUG
    printf("splitting: block size: %uz, req size: %zuz\n", GET_SIZE(HDRP(block)), block_size);
    #endif
    if (split(block, GET_SIZE(HDRP(block)) - block_size) == NULL) {
      printf("split error");
      return NULL;
    }
  }

  // sanity check
  // assert(!GET_ALLOC(HDRP(block)));
  // printf("hdr: %u, ftr: %u\n", GET(HDRP(block)), GET(FTRP(block)));
  // assert(GET(HDRP(block)) ==  GET(FTRP(block)));

  // mark block as allocated
  SET_ALLOC(HDRP(block), 1);

  // mark next block's "prev free" as false
  SET_PREV_FREE(HDRP(NEXT_BLKP(block)), 0);
  // printf("allocated %u at %p\n", GET_SIZE(HDRP(block)), HDRP(block));
  return block;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
  // printf("free called: %p\n", ptr);
  // mark block as deallocated, and set footer
  SET_ALLOC(HDRP(ptr), 0);
  PUT(FTRP(ptr), GET(HDRP(ptr)));

  coalesce(ptr);
  return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
  printf("realloc called\n");
  if (ptr == NULL) {
    return mm_malloc(size);
  }
  void *new_block = mm_malloc(size);
  if (!new_block) {
    return NULL;
  }
  memcpy(new_block, ptr, GET_SIZE(HDRP(ptr)));
  mm_free(ptr);
  return NULL;
}
