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

struct PhysicalMMap {
   size_t total_ram;
   unsigned int num_mmap;
   struct MMapEntry ram_sects[MAX_MMAP_ENTRIES];

   unsigned int num_kernel_sects;
   struct KernelSection kernel_sects[MAX_KERNEL_SECTIONS];
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"

#ifndef offsetof
#define offsetof(a,b) ((offset_t)(&(((a*)(0))->b)))
#endif

static inline offset_t ptr_to_int(void *p)
{
   return (offset_t)p;
}

static inline void *int_to_ptr(offset_t o)
{
   return (void *)o;
}

#pragma GCC diagnostic pop


#endif