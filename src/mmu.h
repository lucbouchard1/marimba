#ifndef __MMU_H__
#define __MMU_H__

#define MMU_PAGE_SIZE 0x1000

int MMU_init(struct SystemMMap *map);
void *MMU_pf_alloc();
void MMU_pf_free(void *pf);

#endif