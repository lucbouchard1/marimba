#ifndef __MMU_H__
#define __MMU_H__

#include "paging.h"

int MMU_init(struct PhysicalMMap *map);
void *MMU_pf_alloc();
void MMU_pf_free(void *pf);
void MMU_stress_test();

#endif