#ifndef __UTILS_H__
#define __UTILS_H__

#include "types.h"

/**
 * Compare function - 
 *    return -1 if first arg is greater
 *    return  0 if args are equal
 *    return  1 if second arg is greater
 */
void sort(void *arr, size_t len, size_t el_len,
      int (*cmp)(void *arg1, void *arg2),
      void (*swp)(void *arg1, void *arg2));

void sort_kernel_section_array(struct KernelSection *arr, size_t len);
void sort_mmap_entry_array(struct MMapEntry *arr, size_t len);

#define max(x,y) ( \
    { __auto_type __x = (x); __auto_type __y = (y); \
      __x > __y ? __x : __y; })

#endif