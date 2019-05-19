#ifndef __MMU_H__
#define __MMU_H__

#include "paging.h"

int MMU_init(struct PhysicalMMap *map);
void *MMU_alloc_frame();
void MMU_free_frame(void *addr);
void *MMU_alloc_page();
void MMU_free_page(void *addr);
void MMU_stress_test();

#endif