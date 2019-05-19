#ifndef __MMU_H__
#define __MMU_H__

#include "paging.h"

int MMU_init(struct PhysicalMMap *map);
void *MMU_frame_alloc();
void MMU_frame_free(void *pf);
void MMU_stress_test();

#endif