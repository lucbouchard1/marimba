#ifndef __PAGING_H__
#define __PAGING_H__

#include "types.h"

void PT_init(struct SystemMMap *map);
void PT_page_table_init(void *addr);

#if ARCH == x86_64
#include "arch/x86_64/paging.h"
#else
#error
#endif

#endif
