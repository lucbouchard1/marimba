#include "../../paging.h"
#include "../../string.h"
#include "../../interrupts.h"
#include "../../printk.h"
#include "../../mmap.h"
#include "../../mmu.h"
#include "../../klog.h"

#include <stdint.h>

#define HUGE_PAGE_FRAME_SIZE 0x200000
#define PAGE_TABLE_DEPTH 3
#define PHYSICAL_PAGE_DEPTH 4

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
   return int_to_ptr(ent->address << 12);
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

   if (depth == PHYSICAL_PAGE_DEPTH)
      return addr & 0xFFF;
   return (addr >> (12 + 9*(PAGE_TABLE_DEPTH - depth))) & 0x1FF;
}

static inline struct PTE *pt_get_next_pte(struct PTE *pt, void *vaddr, int depth)
{
   if (!pt)
      pt = pt_get_addr();
   else
      pt = pte_get_addr(pt);

   return &pt[pt_offset_for_depth(vaddr, depth)];
}

static int pt_walk(void *vaddr, struct PTE **dest)
{
   struct PTE *curr = NULL;
   int i;

   for (i = 0; i < PHYSICAL_PAGE_DEPTH; i++) {
      curr = pt_get_next_pte(curr, vaddr, i);
      if (!curr->present) {
         *dest = curr;
         return i;
      }
   }

   *dest = pt_get_next_pte(curr, vaddr, i);
   return PHYSICAL_PAGE_DEPTH;
}

static void page_fault_handler(int irq, int err, void *arg)
{
   void *req_addr, *frame;
   struct PTE *ent;
   int depth;

   req_addr = pt_get_req_vaddr();

   klog(KLOG_LEVEL_DEBUG, "page fault on address %p. Error %x", req_addr, err);

   if ((depth = pt_walk(req_addr, &ent)) == PHYSICAL_PAGE_DEPTH) {
      klog(KLOG_LEVEL_WARN, "invalid page fault");
      return;
   }

   if (!ent->demand_allocate || depth != PAGE_TABLE_DEPTH) {
      klog(KLOG_LEVEL_WARN, "unhandled page fault");
      return;
   }

   frame = MMU_alloc_frame();
   if (!frame) {
      klog(KLOG_LEVEL_CRIT, "out of memory");
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
      pte_set_addr(&identity_p2[i], int_to_ptr(i*HUGE_PAGE_FRAME_SIZE));
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

int PT_demand_allocate(void *vaddr)
{
   struct PTE *ent;
   void *new_pt;
   int depth;

   depth = pt_walk(vaddr, &ent);
   if (depth == PHYSICAL_PAGE_DEPTH) {
      klog(KLOG_LEVEL_WARN, "allocation attempted twice on virtual address %p", vaddr);
      return -1;
   }

   for (; depth < PAGE_TABLE_DEPTH; depth++) {
      new_pt = MMU_alloc_frame();
      if (!new_pt) {
         klog(KLOG_LEVEL_CRIT, "out of memory");
         return -1;
      }
      init_empty_page_table(new_pt);
      pte_set_addr(ent, new_pt);
      ent = pt_get_next_pte(ent, vaddr, depth+1);
   }
   ent->demand_allocate = 1;

   return 0;
}

void *PT_free(void *vaddr)
{
   int i;
   struct PTE *curr = NULL;

   if (ptr_to_int(vaddr) % PAGE_SIZE) {
      klog(KLOG_LEVEL_WARN, "attempting to free invalid address");
      return NULL;
   }

   for (i = 0; i < PHYSICAL_PAGE_DEPTH; i++) {
      curr = pt_get_next_pte(curr, vaddr, i);
      if (!curr->present)
         return NULL;
   }

   curr->present = 0;
   curr->demand_allocate = 0;
   return pt_get_next_pte(curr, vaddr, i);
}


void *PT_addr_virt_to_phys(void *vaddr)
{
   struct PTE *addr;

   if (pt_walk(vaddr, &addr) == PHYSICAL_PAGE_DEPTH)
      return addr;
   return NULL;
}
