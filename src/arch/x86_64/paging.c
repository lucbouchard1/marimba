#include "../../paging.h"
#include "../../string.h"

#include <stdint.h>

#define HUGE_PAGE_FRAME_SIZE 0x200000

struct PTE {
   uint64_t present: 1;
   uint64_t writable: 1;
   uint64_t user_accesible: 1;
   uint64_t write_through_caching: 1;
   uint64_t disable_cache: 1;
   uint64_t accessed: 1;
   uint64_t dirty: 1;
   uint64_t huge_page: 1;
   uint64_t global: 1;
   uint64_t available1: 3;
   uint64_t address: 40;
   uint64_t available2: 11;
   uint64_t no_execute: 1;
} __attribute__((packed));

static struct PTE identity_p3[PAGE_TABLE_NUM_ENTS] __attribute__ ((aligned (4096)));
static struct PTE identity_p2[PAGE_TABLE_NUM_ENTS] __attribute__ ((aligned (4096)));

void PT_init(struct SystemMMap *map)
{
   int i;

   /* Initialize third level identity page table */
   memset(identity_p3, 0, sizeof(struct PTE)*PAGE_TABLE_NUM_ENTS);
   identity_p3[0].present = 1;
   identity_p3[0].writable = 1;
   identity_p3[0].address = (uint64_t)identity_p2;

   /* Initialize second level identity page table */
   for (i = 0; i < PAGE_TABLE_NUM_ENTS; i++) {
      identity_p2[i].present = 1;
      identity_p2[i].writable = 1;
      identity_p2[i].address = i*HUGE_PAGE_FRAME_SIZE;
   }
}

void PT_page_table_init(void *addr)
{
   struct PTE *p4_addr = (struct PTE *)addr;

   memset(p4_addr, 0, sizeof(struct PTE)*PAGE_TABLE_NUM_ENTS);
   p4_addr[0].present = 1;
   p4_addr[0].writable = 1;
   p4_addr[0].address = (uint64_t)identity_p3;
}