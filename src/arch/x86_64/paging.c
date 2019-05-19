#include "../../paging.h"
#include "../../string.h"
#include "../../interrupts.h"
#include "../../printk.h"

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

static void page_fault_handler(int irq, int err, void *arg)
{
   void *req_addr;
   void *p4_addr;

   asm volatile ("mov %0, %%cr2" : "=r"(req_addr));
   asm volatile ("mov %0, %%cr3" : "=r"(p4_addr));

   printk("Page fault on address %p. Page table at %p\n", req_addr, p4_addr);

   asm("hlt;");
}

void PT_init(struct PhysicalMMap *map)
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
      identity_p2[i].huge_page = 1;
      identity_p2[i].address = i*HUGE_PAGE_FRAME_SIZE;
   }

   IRQ_set_handler(PAGE_FAULT_IRQ, page_fault_handler, NULL);
}

void PT_page_table_init(void *addr)
{
   struct PTE *p4_addr = (struct PTE *)addr;

   memset(p4_addr, 0, sizeof(struct PTE)*PAGE_TABLE_NUM_ENTS);
   p4_addr[0].present = 1;
   p4_addr[0].writable = 1;
   p4_addr[0].address = (uint64_t)identity_p3;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
void *PT_addr_virt_to_phys(void *p4_addr, void *raw_vaddr)
{
   int offset, i;
   void **curr = p4_addr;
   uint64_t vaddr = (uint64_t)raw_vaddr;
   uint64_t offset_mask = (uint64_t)0xFF << 39;
   uint64_t page_offset_mask = (uint64_t)0xFFF << 35;

   for (i = 0; i < 4; i++) {
      offset = (vaddr & offset_mask) >> 39;
      curr = (void *)((struct PTE *)(curr[offset]))->address;
      if (!((struct PTE *)curr)->present)
         return NULL;
      vaddr <<= 8;
   }

   return (void *)((uint8_t *)curr + ((vaddr & page_offset_mask) >> 35));
}
#pragma GCC diagnostic pop