#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

typedef unsigned long size_t;

#define MAX_MMAP_ENTRIES 32

struct SystemMMap {
   unsigned int num_mmap;
   struct SystemMMapEntry {
      void *base;
      size_t length;
   } free_entries [MAX_MMAP_ENTRIES];
};

#endif