#ifndef __MULTIBOOT_H__
#define __MULTIBOOT_H__

#include <stdint.h>

#define MAX_MULTIBOOT_MMAP_ENTRIES 32

struct MultibootMMAP {
   unsigned int num_entries;
   struct MultibootMMAPEntry {
      void *base;
      uint64_t length;
   } free_entries [MAX_MULTIBOOT_MMAP_ENTRIES];
};

int MB_parse_multiboot(uint32_t mb_magic, uint32_t mb_addr);

#endif