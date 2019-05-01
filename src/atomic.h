#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#if ARCH == x86_64
#include "arch/x86_64/atomic.h"
#else
#error
#endif

void atomic_add(volatile atomic_t *mem, int add);
void atomic_sub(volatile atomic_t *mem, int sub);
int atomic_get(volatile atomic_t *mem);

#endif