#ifndef __X86_64_PAGING_H__
#define __X86_64_PAGING_H__

#define PAGE_SIZE 0x1000
#define PAGE_TABLE_NUM_ENTS 512
#define PAGE_TABLE_SIZE (sizeof(void *) * PAGE_TABLE_NUM_ENTS)

static inline void PT_change(void *p4_addr)
{
   asm volatile ("mov %0, %%cr3" : : "r"(p4_addr));
}

#endif