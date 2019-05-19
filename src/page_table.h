#ifndef __PAGE_TABLE_H__
#define __PAGE_TABLE_H__

#include "types.h"

void PT_init(struct SystemMMap *map);
void PT_page_table_init(void *addr);

#if ARCH == x86_64
#include "arch/x86_64/page_table.h"
#else
#error
#endif

#endif