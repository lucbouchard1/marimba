#ifndef __MMU_H__
#define __MMU_H__

#include "paging.h"

int MMU_init(struct PhysicalMMap *map);
void *MMU_alloc_frame();
void MMU_free_frame(void *addr);
void *MMU_alloc_page();
void *MMU_alloc_pages(int num_pages);
void MMU_free_page(void *addr);
void MMU_free_pages(void *addr, int num_pages);
void *MMU_alloc_stack();

#endif