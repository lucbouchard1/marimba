#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include "types.h"

void kfree(void *addr);
void *kmalloc(size_t size);

#endif