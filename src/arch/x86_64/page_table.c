#include "../../page_table.h"
#include "../../types.h"

#include <stdint.h>

#define PAGE_TABLE_SIZE 512
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
   uint64_t available: 3;
   uint64_t address: 40;
   uint64_t available: 11;
   uint64_t no_execute: 1;
} __attribute__((packed));

static struct PTE identity_p3[PAGE_TABLE_SIZE] __attribute__ ((aligned (4096)));
static struct PTE identity_p2[PAGE_TABLE_SIZE] __attribute__ ((aligned (4096)));

void PT_init(struct SystemMMap *map)
{
   int i;

   /* Initialize third level identity page table */
   memset(identity_p3, 0, sizeof(struct PTE)*PAGE_TABLE_SIZE);
   identity_p3[0].present = 1;
   identity_p3[0].writable = 1;
   identity_p3[0].address = (uint64_t)identity_p2;

   /* Initialize second level identity page table */
   for (i = 0; i < PAGE_TABLE_SIZE; i++) {
      identity_p2[i].present = 1;
      identity_p2[i].writable = 1;
      identity_p2[i].address = i*HUGE_PAGE_FRAME_SIZE;
   }
}