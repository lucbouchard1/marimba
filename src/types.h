#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned long size_t;
typedef unsigned long offset_t;

#define MAX_MMAP_ENTRIES 32
#define MAX_KERNEL_SECTIONS 32

struct MMapEntry {
   void *base;
   size_t length;
};

struct KernelSection {
   void *base;
   size_t length;
   const char *section_name;
};

struct SystemMMap {
   size_t total_ram;
   unsigned int num_mmap;
   struct MMapEntry avail_ram[MAX_MMAP_ENTRIES];

   unsigned int num_kernel_sects;
   struct KernelSection kernel_sects[MAX_KERNEL_SECTIONS];
};

#endif