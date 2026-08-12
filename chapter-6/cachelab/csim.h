#include <stdint.h>

typedef struct LRU_Node {
  uint64_t tag;
  struct LRU_Node *next;
  struct LRU_Node *prev;
} LRU_Node;

typedef struct {
  // next in line for eviction
  LRU_Node *head;
  // least recently used
  LRU_Node *tail;
  // array of nodes
  LRU_Node *nodes;
} LRU;


typedef struct {
  // amount of items in the set
  uint32_t len;
  union {
    uint64_t tag;
    LRU lru;
  } inner;
} Set;

typedef struct Cache {
  uint32_t hits;
  uint32_t misses;
  uint32_t evictions;

  // array of sets
  Set *sets;
} Cache;

void set_load(Cache *cache, uint32_t set_idx, uint64_t tag, uint32_t assoc);

Cache new_cache(int set_bits, int assoc);
void process_instruction(char *line, Cache *cache, uint32_t set_bits,
                         uint32_t block_bits, uint32_t assoc);
