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
   uint64_t demand_allocate: 1;
   uint64_t available1: 2;
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

static inline void pte_set_addr(struct PTE *ent, void *addr)
{
   *((void **)ent) = addr;
   ent->present = 1;
   ent->writable = 1;
}

static inline void *pte_get_addr(struct PTE *ent)
{
   return (void *)(ent->address << 12);
}

static inline void *pt_get_addr()
{
   void *p4_addr;
   asm volatile ("mov %%cr3, %0" : "=r"(p4_addr));
   return p4_addr;
}

static inline void *pt_get_req_vaddr()
{
   void *req_addr;
   asm volatile ("mov %%cr2, %0" : "=r"(req_addr));
   return req_addr;
}

static inline int pt_offset_for_depth(void *vaddr, int depth)
{
   uint64_t addr = (uint64_t)vaddr;

   if (depth == PAGE_TABLE_DEPTH)
      return addr & 0xFFF;
   return (addr >> (12 + 9*(PAGE_TABLE_DEPTH - depth - 1))) & 0x1FF;
}

static int pt_walk(void *p4_addr, void *vaddr, struct PTE **dest)
{
   int offset, i;
   struct PTE *curr = p4_addr;

   for (i = 0; i < PAGE_TABLE_DEPTH; i++) {
      offset = pt_offset_for_depth(vaddr, i);
      if (!curr[offset].present) {
         *dest = &curr[offset];
         return i;
      }
      curr = pte_get_addr(&curr[offset]);
   }

   *dest = (struct PTE *)((uint8_t *)curr + pt_offset_for_depth(vaddr, i));
   return PAGE_TABLE_DEPTH;
}

static void page_fault_handler(int irq, int err, void *arg)
{
   void *req_addr, *p4_addr, *frame;
   struct PTE *ent;
   int depth;
   // Need to check if address is in identity mapped region!! <<< IMPORTANT

   req_addr = pt_get_req_vaddr();
   p4_addr = pt_get_addr();

   printk("Page fault on address %p. Page table at %p. Error %x\n", req_addr, p4_addr, err);

   if ((depth = pt_walk(p4_addr, req_addr, &ent)) == PAGE_TABLE_DEPTH) {
      printk("error: invalid page fault\n");
      return;
   }

   printk("Entry Addr %p\n", ent);

   if (!ent->demand_allocate || depth != PAGE_TABLE_DEPTH - 1) {
      printk("error: unhandled page fault\n");
      return;
   }

   frame = MMU_alloc_frame();
   if (!frame) {
      printk("error: out of memory\n");
      return;
   }
   pte_set_addr(ent, frame);
}

void PT_init(struct PhysicalMMap *map)
{
   int i;

   /* Initialize third level identity page table */
   init_empty_page_table(identity_p3);
   pte_set_addr(identity_p3, identity_p2);

   /* Initialize second level identity page table */
   for (i = 0; i < PAGE_TABLE_NUM_ENTS; i++) {
      pte_set_addr(&identity_p2[i], (void *)(i*HUGE_PAGE_FRAME_SIZE));
      identity_p2[i].huge_page = 1;
   }

   IRQ_set_handler(PAGE_FAULT_IRQ, page_fault_handler, NULL);
}

void PT_page_table_init(void *addr)
{
   struct PTE *p4_addr = (struct PTE *)addr;

   init_empty_page_table(p4_addr);
   pte_set_addr(p4_addr, identity_p3);
}
#pragma GCC diagnostic pop

int PT_demand_allocate(void *vaddr)
{
   struct PTE *ent;
   void *p4_addr = pt_get_addr(), *new_pt;
   int depth;

   depth = pt_walk(p4_addr, vaddr, &ent);
   if (depth >= PAGE_TABLE_DEPTH-1) {
      printk("error: allocation attempted twice on virtual address %p\n", vaddr);
      return -1;
   }

   for (; depth < PAGE_TABLE_DEPTH-1; depth++) {
      new_pt = MMU_alloc_frame();
      if (!new_pt) {
         printk("error: out of memory\n");
         return -1;
      }
      init_empty_page_table(new_pt);
      pte_set_addr(ent, new_pt);
      ent = ((struct PTE *)new_pt) + pt_offset_for_depth(vaddr, depth+1);
   }
   ent->demand_allocate = 1;

   return 0;
}


void *PT_addr_virt_to_phys(void *vaddr)
{
   struct PTE *addr;
   void *p4_addr = pt_get_addr();

   if (pt_walk(p4_addr, vaddr, &addr) == PAGE_TABLE_DEPTH)
      return addr;
   return NULL;
}
