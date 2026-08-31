/*
 * memory allocator with explicit free list
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define IS_ALIGNED(bp) ((((size_t)bp) & ~(0x7)) == 0 ? 1 : 0)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

_Static_assert(sizeof(void *) == 4, "pointer isn't 32-bit");
_Static_assert(sizeof(unsigned int) == 4, "unsigned int isn't 32-bit");

/* Basic constants and macros */
#define WSIZE 4             /* Word and header/footer size () */
#define DSIZE 8             /* Double word size () */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */
#define BLOCK_MIN_SIZE (4 * WSIZE)
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

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
// the second bit marks whether the previous page is deallocted #define
// GET_PREV_FREE(p) ((GET(p) & 0x2) > 0 ? 1 : 0)
#define GET_PREV_FREE(p) ((GET(p) & 0x2) > 0 ? 1 : 0)
/* Sets the pointed at hdr prev_free field */
#define SET_PREV_FREE(p, prev_free)                                            \
  (PUT(p, ((GET(p) & ~0x2) | (prev_free << 1))))

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Block size needed for a request, the request size plus size for hdr and
 * footer rounded to alignment*/
#define BLOCK_SIZE(req_size) (ALIGN(req_size + WSIZE))
/* Checks if the block is the epilogue block */
#define IS_END(bp) (GET_SIZE(HDRP(bp)) == 0 && GET_ALLOC(HDRP(bp)) == 1 ? 1 : 0)

#define BLOCK_TO_BP(block_addr) ((void *)((char *)(block_addr) + WSIZE))
#define BP_TO_BLOCK(bp) ((Block *)((char *)(bp) - WSIZE))

// macros for doubly linked list
#define GET_NEXT(block) (GET(block))
#define SET_NEXT(block, np) (PUT(block, (unsigned int)(np)))
#define GET_PREV(block) (GET((char *)(block) + WSIZE))
#define SET_PREV(block, pp) (PUT(((char *)(block) + WSIZE), (unsigned int)(pp)))

// new block api
#define NEXT_BLOCK(block) ((char*)(block) + GET_SIZE(block)))
#define PREV_BLOCK(block) ((char*)(block) + GET_SIZE((char*)(block) - WSIZE))
#define FTR_PTR(block) ((char *)(block) + GET_SIZE(block) - WSIZE)
#define SET_FTR(block, val) (PUT(FTR_PTR(block)), val)

#define IS_ALLOC(block) (GET_ALLOC(block))
#define IS_PREV_FREE(block) (GET_PREV_FREE(block))

#define SET_BP_ALLOC(bp, alloc) (SET_ALLOC(HDRP(bp), alloc))
#define SET_BP_PREV_ALLOC(bp, alloc) (SET_ALLOC(HDRP(bp), alloc))

// head of the list, points to block
void *list_head;
char *heap_end;

// next and prev are defined as bp
typedef struct Block {
  unsigned int hdr;
  void *next;
  void *prev;
} Block;

unsigned int is_alloc(Block *block);
unsigned int prev_alloc(Block *block);
unsigned int get_size(Block *block);

Block new_block(size_t size, unsigned int prev_alloc, unsigned int alloc);
void write_block(void *bp, Block block);
void disc_bp(void *bp);

void prepend_list(void **list, void *bp);

void *find_free_block(void *bp, size_t block_size);
void *grow_heap(size_t size);
void *coalesce(void *bp);
void *split(void *bp, size_t block_size);

Block new_block(size_t size, unsigned int prev_free, unsigned int alloc) {
  assert(size % 8 == 0);

  Block bl;
  bl.hdr = size | (!!prev_free << 1) | !!alloc;
  bl.next = 0;
  bl.prev = 0;
  return bl;
};

// disconnects and prepends block at bp
// list points to a bp
void prepend_list(void **list, void *bp) {
  assert(bp != NULL);
  assert(!GET_ALLOC(HDRP(bp)));

  if (*list == NULL) {
    *list = bp;
    return;
  }
  assert((char *)*list < heap_end);

  Block *new_bl = BP_TO_BLOCK(bp);
  Block *old_head = BP_TO_BLOCK(*list);

  if (new_bl == old_head) {
    return;
  }

  // disc_bp(bp);

  old_head->prev = bp;
  new_bl->next = *list;
  new_bl->prev = 0;
  *list = bp;
  return;
};

// write block to bp
void write_block(void *bp, Block block) {
  PUT(HDRP(bp), block.hdr);
  if (!GET_ALLOC(HDRP(bp))) {
    PUT(FTRP(bp), block.hdr);
    SET_PREV_FREE(HDRP(NEXT_BLKP(bp)), 1);
  }
  SET_NEXT(bp, 0);
  SET_PREV(bp, 0);
  return;
};

// disconnects a block and reconnects the next and previous blocks
// should only be called on free blocks
void disc_bp(void *bp) {
  assert(bp != NULL);
  assert(!GET_ALLOC(HDRP(bp)));

  // handling disconnecting the head
  if (bp == list_head) {
    if (GET_NEXT(bp) == 0 && GET_PREV(bp) == 0) {
      list_head = NULL;
    } else if (GET_NEXT(bp)) {
      list_head = (void *)GET_NEXT(bp);
      SET_PREV(GET_NEXT(bp), 0);
    }
    SET_NEXT(bp, 0);
    SET_PREV(bp, 0);
    return;
  }

  if (GET_NEXT(bp) != 0) {
    SET_PREV(GET_NEXT(bp), GET_PREV(bp));
  }
  if (GET_PREV(bp) != 0) {
    SET_NEXT(GET_PREV(bp), GET_NEXT(bp));
  }
  SET_NEXT(bp, 0);
  SET_PREV(bp, 0);
  return;
};

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
  assert(sizeof(Block) + WSIZE == BLOCK_MIN_SIZE);
  // get a start allocation
  if (mem_sbrk(WSIZE * 4) == (void *)-1) {
    return -1;
  }

  list_head = mem_heap_lo();
  // end points to one over the max heap
  heap_end = list_head + WSIZE * 4;

  // we set the start block to size 8 and allocated
  PUT(list_head + WSIZE, PACK(DSIZE, 0, 1));

  // end block has special sentinal value of allocated without a size
  PUT(list_head + (WSIZE * 3), PACK(0, 0, 1));

  list_head = NULL;

  // assert(!IS_ALIGNED(list_head));
  return 0;
}

// grows the heap by size bytes, returns block pointer to new free block
void *grow_heap(size_t size) {
  void *new;
  int prev_free;

  if (size < BLOCK_MIN_SIZE) {
    printf("min size grow heap error\n");
    return NULL;
  }

  // round up to multiples of 8 and make space for epilogue header
  size = ALIGN(size);

  if ((new = mem_sbrk(size)) == (void *)-1) {
    printf("sbrk error\n");
    return NULL;
  }

  // sanity check
  assert(new == heap_end);
  assert(IS_END(new));

  // new free block, with previous "prev free" flag carried over
  prev_free = GET_PREV_FREE(HDRP(new));
  write_block(new, new_block(size, prev_free, 0));

  // new epilogue, with previous block marked as free
  heap_end = new + size;
  PUT(HDRP(heap_end), PACK(0, 1, 1));

  return coalesce(new);
}

// coalesces next and previous block if appropiate, takes a block pointer
// this function assumes its being called on a deallocted block
// that is not in the list!
// returns the pointer to the coalesced block
void *coalesce(void *bp) {
  assert(!IS_END(bp));
  assert(!GET_ALLOC(HDRP(bp)));
  assert(GET_NEXT(bp) == 0);
  assert(GET_PREV(bp) == 0);

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
    disc_bp(nxt_bl);
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
    assert(!GET_ALLOC(HDRP(prev_bl)));
    assert(!GET_PREV_FREE(HDRP(prev_bl)));

    // add sizes together, and write new values
    new_size = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(prev_bl));
    PUT(HDRP(prev_bl), PACK(new_size, 0, 0));
    PUT(FTRP(prev_bl), PACK(new_size, 0, 0));

    // update the return pointer
    bp = prev_bl;
  }

  if (!prev_free) {
    // we only append to list if we didnt coalesce at all, or coalesced with the
    // next block onyl
    prepend_list(&list_head, bp);
  }

  return bp;
}

// walk the list and find a "first fit" block
//
// returns null if the heap is full or it cant find an appropiate block
void *find_free_block(void *bp, size_t req_size) {
  assert(req_size % 8 == 0);

  while (bp != NULL && !IS_END(bp) && (char *)bp < heap_end) {
    if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= req_size) {
#ifdef DEBUG
      printf("found block at: %p for req_size: %zu\n", bp, req_size);
#endif
      return bp;
    }
    bp = (void *)GET_NEXT(bp);
  }
  return NULL;
}

/// splits off split_n from block bp
/// the split block header has the same flags but different size
/// returns pointer to split off block, or NULL on error
void *split(void *bp, size_t split_n) {
  assert(split_n % 8 == 0);
  void *split_block;
  size_t old_size = GET_SIZE(HDRP(bp));

  // we cant split the prologue block
  if (IS_END(bp)) {
    return NULL;
  }

  if (old_size == split_n)
    return bp;

  // we cant carve off less than minimum block size
  if (split_n < BLOCK_MIN_SIZE)
    return NULL;

  // we cant carve off more than the block has
  if (old_size < split_n)
    return NULL;

  // the remaining block cant be smaller than min_block_size
  if ((old_size - split_n) < BLOCK_MIN_SIZE)
    return NULL;
  ;

  // update old header
  SET_SIZE(HDRP(bp), old_size - split_n);

  // in case we are splitting a free block
  if (!GET_ALLOC(HDRP(bp))) {
    PUT(FTRP(bp), GET(HDRP(bp)));
  }

  // calculate offset, and write new split off block
  split_block = (char *)bp + (old_size - split_n);
  write_block(split_block, new_block(split_n, GET_PREV_FREE(HDRP(bp)), 0));

  // we might split off of realloc, so we need to account for a new 
  // free block appearing
  return coalesce(split_block);
};

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t req_size) {
#ifdef DEBUG
  printf("malloc called: %zu\n", req_size);
#endif // DEBUG
  void *block;
  size_t block_size;

  if (req_size == 0) {
    return NULL;
  }

  // ensure we always allocate at least BLOCK_MIN_SIZE for every request
  // smaller than that
  if (req_size < BLOCK_MIN_SIZE) {
    req_size = BLOCK_MIN_SIZE;
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
    printf("splitting: block size: %uz, req size: %zuz\n",
           GET_SIZE(HDRP(block)), block_size);
#endif
    if (split(block, GET_SIZE(HDRP(block)) - block_size) == NULL) {
      // printf("split error");
      // return NULL;
    }
  }

  disc_bp(block);

  // sanity check
  assert(!GET_ALLOC(HDRP(block)));
#ifdef DEBUG
  printf("hdr: %u, ftr: %u\n", GET(HDRP(block)), GET(FTRP(block)));
#endif
  assert(GET(HDRP(block)) == GET(FTRP(block)));

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
  if (ptr == NULL) {
    return;
  }
  // printf("free called: %p\n", ptr);

  // mark block as deallocated, and set footer
  SET_ALLOC(HDRP(ptr), 0);
  SET_NEXT(ptr, 0);
  SET_PREV(ptr, 0);
  PUT(FTRP(ptr), GET(HDRP(ptr)));
  // set the next block's "free_prev" to true
  SET_PREV_FREE(HDRP(NEXT_BLKP(ptr)), 1);

  coalesce(ptr);
#ifdef DEBUG
  printf("new block: %u\n", GET_SIZE(HDRP(freed)));
#endif
  return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
#ifdef DEBUG
  printf("realloc called for size %zu\n", size);
#endif
  if (size == 0) {
    return NULL;
  }
  if (ptr == NULL) {
    return mm_malloc(size);
  }

  unsigned int block_size = GET_SIZE(HDRP(ptr));

  // if the requested size is smaller then the current block, we truncate
  if (block_size >= BLOCK_SIZE(size)) {
    // if the difference can make for another block we split it
    split(ptr, block_size - BLOCK_SIZE(size));
    return ptr;
  }

  // we have to find a larger block

  // can we coalsce the next block?
  // is the next block free, and does the sum of current block and next block
  // suffice
  if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) &&
      GET_SIZE(HDRP(NEXT_BLKP(ptr))) + block_size >= BLOCK_SIZE(size)) {
    disc_bp(NEXT_BLKP(ptr));
    // update the size
    SET_SIZE(HDRP(ptr), GET_SIZE(HDRP(NEXT_BLKP(ptr))) + block_size);
    // set the block after the next block
    SET_PREV_FREE(HDRP(NEXT_BLKP(ptr)), 0);
    return ptr;
  }

  void *new_block = mm_malloc(size);
  if (!new_block) {
    return NULL;
  }
  memcpy(new_block, ptr, block_size - WSIZE);
  mm_free(ptr);
  return new_block;
}

void mm_checkheap(int verbose) {
  // print freelist
  void *bp = list_head;
  int count = 0;
  Block *bl;
  while (bp != NULL && (char *)bp < heap_end) {
    bl = BP_TO_BLOCK(bp);
    printf("free list entry [%d] at [%p], size: [%u], next: [%p], prev: [%p]\n",
           count, (char *)bl + WSIZE, GET_SIZE(HDRP(BLOCK_TO_BP(bl))), bl->next,
           bl->prev);
    bp = (void *)GET_NEXT(bp);
    count++;
  }
  return;
};
