#ifndef __X86_64_PAGE_TABLE_H__
#define __X86_64_PAGE_TABLE_H__

#define PAGE_TABLE_NUM_ENTS 512
#define PAGE_TABLE_SIZE (sizeof(void *) * PAGE_TABLE_NUM_ENTS)

static inline void PT_change(void *p4_addr)
{
   asm volatile ("mov %%cr3, %0" : : "r"(p4_addr));
}

#endif