#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

typedef unsigned long size_t;
typedef unsigned long offset_t;

#define MAX_MMAP_ENTRIES 32
#define MAX_KERNEL_SECTIONS 32

struct SystemMMap {
   unsigned int num_mmap;
   struct MMapEntry {
      void *base;
      size_t length;
   } avail_ram[MAX_MMAP_ENTRIES];

   unsigned int num_kernel_sects;
   struct KernelSection {
      void *base;
      size_t length;
      const char *section_name;
   } kernel_sects[MAX_KERNEL_SECTIONS];
};

#endif