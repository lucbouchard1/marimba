#ifndef __PAGING_H__
#define __PAGING_H__

#include "types.h"

void PT_init(struct PhysicalMMap *map);
void PT_page_table_init(void *addr);
void *PT_addr_virt_to_phys(void *vaddr);
int PT_demand_allocate(void *vaddr);

#if ARCH == x86_64
#include "arch/x86_64/paging.h"
#else
#error
#endif

#endif
