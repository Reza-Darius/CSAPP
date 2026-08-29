#define _POSIX_C_SOURCE 200809L

#include "csim.h"
#include "cachelab.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ADDR_LEN = 64

/*
 * parse arguments s, e, b
 * allocate memory s^2 * e * b^2, set bits to amount of sets * number of lines
 * per set * block size one LRU per set in case of e > 1 open file parse lines
 * into operation, address, bytes
 *
 * LRU: seperate linked list data structure
 *
 * flag tag with vacant bit
 * Record struct to record misses and hits
 */

Cache new_cache(int set_bits, int assoc) {
  int nsets;
  Cache cache = {0};
  Set *set;

  nsets = pow(2, set_bits);

  cache.sets = calloc(sizeof(Set), nsets);
  if (!cache.sets) {
    printf("calloc for new cache failed\n");
    exit(EXIT_FAILURE);
  };

  // we only need the LRU for a non direct mapped cache
  if (assoc > 1) {
    for (int i = 0; i < nsets; i++) {
      set = &cache.sets[i];

      set->inner.lru.nodes = calloc(sizeof(LRU_Node), assoc);
      if (!set->inner.lru.nodes) {
        printf("calloc for new cache failed\n");
        exit(EXIT_FAILURE);
      };
    }
  }
  return cache;
}

void process_instruction(char *line, Cache *cache, uint32_t set_bits,
                         uint32_t block_bits, uint32_t assoc) {
  uint32_t set_idx;
  uint64_t tag, addr, mask = 0;

  // parse the instruction
  char *addr_str = strtok(&line[3], ",");
  if (!addr_str) {
    printf("couldnt find addr string for strtok, line: %s\n", line);
    exit(EXIT_FAILURE);
  }
  addr = strtol(addr_str, NULL, 16);

  // 0000, s = 2, b = 2, shift by 4 = 10000 - 1 = 01111
  mask = (1 << (set_bits + block_bits)) - 1;

  set_idx = (addr & mask) >> block_bits;
  tag = addr >> (set_bits + block_bits);

  set_load(cache, set_idx, tag, assoc);

  if (line[1] == 'M') {
    // modify is a store load, so while the initial store might miss, the second
    // load is always a hit
    printf("modify hit\n");
    cache->hits++;
  }
  return;
};

void set_load(Cache *cache, uint32_t set_idx, uint64_t tag, uint32_t assoc) {
  Set *set = &cache->sets[set_idx];
  LRU *lru = &set->inner.lru;
  uint32_t len = set->len;

  printf("checking tag %lu, in set %d, len %d\n", tag, set_idx, len);

  if (assoc == 1) {
    if (len == 0) {
      printf("miss\n");
      cache->misses++;
      set->inner.tag = tag;
      set->len++;
    } else if (set->inner.tag == tag) {
      printf("hit\n");
      cache->hits++;
    } else {
      set->inner.tag = tag;
      printf("miss eviction\n");
      cache->misses++;
      cache->evictions++;
    }
    return;
  }

  LRU_Node *node;

  // do we have it in the cache?
  printf("searching cache\n");
  for (node = lru->head; node != NULL; node = node->next) {
    // tag found, we have to promote it now
    if (node->tag == tag) {
      if (node == lru->tail) {
        // we are already LRU
        printf("hit\n");
        cache->hits++;
        return;
      }

      // disconnect node
      if (node == lru->head) {
        node->next->prev = NULL;
        lru->head = node->next;
      } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
      }

      // append to tail
      // connect node
      lru->tail->next = node;
      node->prev = lru->tail;
      node->next = NULL;

      // tail points to newly appended node
      lru->tail = node;

      printf("hit\n");
      cache->hits++;
      return;
    }
  }

  // do we have space to just append
  if (len < assoc) {
    printf("appending to cache\n");
    node = &lru->nodes[len];
    node->tag = tag;

    // connect node
    if (len == 0) {
      lru->head = node;
      lru->tail = node;
    } else {
      lru->tail->next = node;
      node->prev = lru->tail;
      lru->tail = node;
    }

    printf("miss\n");
    cache->misses++;
    set->len++;
    return;
  }

  printf("evicting tag cache\n");
  // we have to evict
  node = lru->head;
  node->tag = tag;

  // disconnect head
  node->next->prev = NULL;
  lru->head = node->next;

  // append to tail
  lru->tail->next = node;
  node->prev = lru->tail;
  node->next = NULL;

  // tail points to newly appended node
  lru->tail = node;

  printf("miss eviction\n");
  cache->misses++;
  cache->evictions++;
  return;
};

int main(int argc, char **argv) {
  int opt, set_bits, associativity, block_bits;
  char *path;
  Cache cache;

  // parse cli flags
  while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
    switch (opt) {
    case 's':
      set_bits = atoi(optarg);
      break;
    case 'E':
      associativity = atoi(optarg);
      break;
    case 'b':
      block_bits = atoi(optarg);
      break;
    case 't':
      path = optarg;
      break;
    case '?':
      printf("returning question mark %c\n", opt);
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  printf("read set bits %d, assoc %d, block bits %d\n", set_bits, associativity,
         block_bits);

  FILE *stream;
  char *line = NULL;
  size_t size = 0;
  ssize_t nread;

  stream = fopen(path, "r");
  if (stream == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  cache = new_cache(set_bits, associativity);

  while ((nread = getline(&line, &size, stream)) != -1) {
    if (line[0] == 'I') {
      continue;
    }
    process_instruction(line, &cache, set_bits, block_bits, associativity);
  }

  printSummary(cache.hits, cache.misses, cache.evictions);

  free(line);
  fclose(stream);
  return EXIT_SUCCESS;
}
