#include "../../paging.h"
#include "../../string.h"
#include "../../interrupts.h"
#include "../../printk.h"
#include "../../mmap.h"
#include "../../mmu.h"

#include <stdint.h>

#define HUGE_PAGE_FRAME_SIZE 0x200000
#define PAGE_TABLE_DEPTH 4

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

static inline void init_empty_page_table(void *pt)
{
   memset(pt, 0, sizeof(struct PTE)*PAGE_TABLE_NUM_ENTS);
}

static inline void pte_load_addr(struct PTE *ent, void *addr)
{
   *((void **)ent) = addr;
   ent->present = 1;
   ent->writable = 1;
}

static int walk_page_table(void *p4_addr, void *raw_vaddr, struct PTE **dest)
{
   int offset, i;
   uint64_t vaddr = (uint64_t)raw_vaddr;
   uint64_t offset_mask = (uint64_t)0xFF << 39;
   uint64_t page_offset_mask = (uint64_t)0xFFF << 35;
   struct PTE *curr = p4_addr;

   for (i = 0; i < PAGE_TABLE_DEPTH; i++) {
      offset = (vaddr & offset_mask) >> 39;
      if (!curr[offset].present) {
         *dest = &curr[offset];
         return i;
      }
      curr = (struct PTE *)(curr[offset].address << 12);
      vaddr <<= 8;
   }

   *dest = (struct PTE *)((uint8_t *)curr + ((vaddr & page_offset_mask) >> 35));
   return PAGE_TABLE_DEPTH;
}

static void page_fault_handler(int irq, int err, void *arg)
{
   void *req_addr, *p4_addr, *frame;
   struct PTE *ent;
   int depth;
   // Need to check if address is in identity mapped region!! <<< IMPORTANT

   asm volatile ("mov %%cr2, %0" : "=r"(req_addr));
   asm volatile ("mov %%cr3, %0" : "=r"(p4_addr));

   printk("Page fault on address %p. Page table at %p\n", req_addr, p4_addr);

   if ((depth = walk_page_table(p4_addr, req_addr, &ent)) == PAGE_TABLE_DEPTH) {
      printk("error: invalid page fault\n");
      return;
   }

   printk("Entry Addr %p\n", ent);
   frame = MMU_alloc_frame();
   if (!frame) {
      printk("error: out of memory\n");
      return;
   }

   if (depth != PAGE_TABLE_DEPTH - 1)
      init_empty_page_table(frame);
   pte_load_addr(ent, frame);
}

void PT_init(struct PhysicalMMap *map)
{
   int i;

   /* Initialize third level identity page table */
   init_empty_page_table(identity_p3);
   pte_load_addr(identity_p3, identity_p2);

   /* Initialize second level identity page table */
   for (i = 0; i < PAGE_TABLE_NUM_ENTS; i++) {
      pte_load_addr(&identity_p2[i], (void *)(i*HUGE_PAGE_FRAME_SIZE));
      identity_p2[i].huge_page = 1;
   }

   IRQ_set_handler(PAGE_FAULT_IRQ, page_fault_handler, NULL);
}

void PT_page_table_init(void *addr)
{
   struct PTE *p4_addr = (struct PTE *)addr;

   init_empty_page_table(p4_addr);
   pte_load_addr(p4_addr, identity_p3);
}

#pragma GCC diagnostic pop

void *PT_addr_virt_to_phys(void *p4_addr, void *raw_vaddr)
{
   struct PTE *addr;

   if (walk_page_table(p4_addr, raw_vaddr, &addr) == PAGE_TABLE_DEPTH)
      return addr;
   return NULL;
}
