#include <stdint.h>
#include "types.h"
#include "mmu.h"
#include "printk.h"
#include "string.h"
#include "utils.h"
#include "mmap.h"
#include "interrupts.h"
#include "klog.h"

#define MAX_FREE_SEGMENTS 32
#define MAX_EXCLUDE_SEGMENTS 128

struct Segment {
   uint8_t *base;
   uint8_t *end;
   size_t len;
};

struct SegmentList {
   struct SegmentList *next;
   struct Segment s;
};

struct MMUState {
   struct SegmentList free_segments[MAX_FREE_SEGMENTS];
   struct SegmentList *free_segment_head;
   void *free_page_head;
   void *free_page_tail;
   void *page_table;
   uint8_t *next_kernel_heap_vaddr;
   uint8_t *next_kernel_thread_stack_vaddr;
};

static struct MMUState mmu_state;

/**
 * Converts the KernelSection entries in the PhysicalMMap into a
 * page-aligned linked list of used memory regions.
 * 
 * @retval Head pointer to the linked list.
 */
static struct SegmentList *mmu_compute_excluded(struct PhysicalMMap *map,
      struct SegmentList *excluded)
{
   struct SegmentList *ex_head, *prev, *curr;
   uint8_t *new_base;
   int i;

   /* Copy kernel segments into excluded segments array */
   ex_head = &excluded[0];
   for (i = 0, prev = NULL; i < map->num_kernel_sects; i++) {
      excluded[i].s.base = map->kernel_sects[i].base;
      excluded[i].s.len = map->kernel_sects[i].length;
      excluded[i].s.end = excluded[i].s.base + excluded[i].s.len;
      excluded[i].next = NULL;
      if (prev)
         prev->next = &excluded[i];
      prev = &excluded[i];
   }

   /* Round excluded sections to page boundaries */
   for (curr = ex_head; curr; curr = curr->next) {
      /* Round down base to page offset */
      new_base = (void *)((uint64_t)curr->s.base - ((uint64_t)curr->s.base % PAGE_SIZE));
      curr->s.len += curr->s.base - new_base;
      curr->s.base = new_base;
      /* Round up length to page size */
      if (curr->s.len % PAGE_SIZE) {
         curr->s.len += PAGE_SIZE;
         curr->s.len -= (curr->s.len % PAGE_SIZE);
      }
      curr->s.end = curr->s.base + curr->s.len;
   }

   /* Reduce overlapping kernel segments into contiguous segments */
   for (curr = ex_head->next, prev = ex_head; curr; prev = curr, curr = curr->next) {
      if (prev->s.base + prev->s.len < curr->s.base)
         continue;

      prev->s.len = max(prev->s.len, (curr->s.base + curr->s.len) - prev->s.base);
      prev->s.end = prev->s.base + prev->s.len;
      prev->next = curr->next;
      curr = prev;
   }

   return ex_head;
}

/**
 * Inverts the linked list of excluded regions into a linked list of
 * free memory regions. Uses the MMap entries in the PhysicalMMap
 */
static void mmu_compute_free_segments(struct MMUState *mmu, struct PhysicalMMap *map,
      struct SegmentList *ex_head)
{
   struct SegmentList *prev, *curr;
   struct SegmentList *fcurr, *ecurr;
   uint64_t new_base;
   int i;

   /* Copy RAM MMap into free_segments array */
   mmu->free_segment_head = mmu->free_segments;
   for (i = 0, prev = NULL; i < map->num_mmap; i++) {
      curr = &mmu->free_segments[i];
      curr->s.base = map->ram_sects[i].base;
      curr->s.len = map->ram_sects[i].length;
      curr->s.end = curr->s.base + curr->s.len;
      curr->next = NULL;
      if (prev)
         prev->next = curr;
      prev = curr;
   }

   /* Round free sections to page boundaries */
   for (curr = mmu->free_segment_head, prev = NULL; curr; curr = curr->next) {
      /* Round up base to page offset */
      new_base = (uint64_t)curr->s.base;
      if (new_base % PAGE_SIZE) {
         new_base += PAGE_SIZE;
         new_base -= (new_base % PAGE_SIZE);
      }
      curr->s.len = (uint64_t)curr->s.end - new_base;
      curr->s.base = (void *)new_base;
      /* Round down length to page size */
      curr->s.len = curr->s.len - (curr->s.len % PAGE_SIZE);
      curr->s.end = curr->s.base + curr->s.len;
      /* Delete free segment if it shrunk too much */
      if (curr->s.len <= 0) {
         if (prev)
            prev->next = curr->next;
         else
            mmu->free_segment_head = curr->next;
      }
      prev = curr;
   }

   /* Remove excluded sections from free segments array */
   prev = NULL;
   for (fcurr = mmu->free_segment_head, ecurr = ex_head; fcurr && ecurr; ) {
      if (fcurr->s.base < ecurr->s.base && fcurr->s.end > ecurr->s.base) {
         fcurr->s.len = ecurr->s.base - fcurr->s.base;
         if (fcurr->s.end > ecurr->s.end) {
            /* Insert new free entry */
            mmu->free_segments[i].s.base = ecurr->s.end;
            mmu->free_segments[i].s.len = fcurr->s.end - ecurr->s.end;
            mmu->free_segments[i].next = fcurr->next;
            fcurr->next = &mmu->free_segments[i];
            i++;
            ecurr = ecurr->next;
         }
         prev = fcurr;
         fcurr = fcurr->next;
      } else if (fcurr->s.base >= ecurr->s.base && fcurr->s.base < ecurr->s.end) {
         if (fcurr->s.end < ecurr->s.end) {
            /* Delete the free entry */
            if (prev)
               prev->next = fcurr->next;
            else
               mmu->free_segment_head = fcurr->next;
         } else {
            fcurr->s.len = fcurr->s.end - ecurr->s.end;
            fcurr->s.base = ecurr->s.end;
            ecurr = ecurr->next;
         }
      } else {
         prev = fcurr;
         fcurr = fcurr->next;
      }
   }

   /* Ignore page at address 0 so NULL checks can work in MMU code */
   if (mmu->free_segment_head->s.base == NULL) {
      if (mmu->free_segment_head->s.len == PAGE_SIZE) {
         mmu->free_segment_head = mmu->free_segment_head->next;
      } else {
         mmu->free_segment_head->s.base += PAGE_SIZE;
         mmu->free_segment_head->s.len -= PAGE_SIZE;
      }
   }
}

int MMU_init(struct PhysicalMMap *map)
{
   struct SegmentList excluded[MAX_EXCLUDE_SEGMENTS];
   struct SegmentList *ex_head;

   memset(&mmu_state, 0, sizeof(struct MMUState));

   if (map->num_kernel_sects > MAX_EXCLUDE_SEGMENTS) {
      klog(KLOG_LEVEL_EMERG, "too many kernel segments for MMU");
      return -1;
   }

   ex_head = mmu_compute_excluded(map, excluded);
   mmu_compute_free_segments(&mmu_state, map, ex_head);

   mmu_state.next_kernel_heap_vaddr = (void *)(MMAP_KERNEL_HEAP_START);
   mmu_state.next_kernel_thread_stack_vaddr = (void *)(MMAP_KERNEL_STACKS_END-8);
   mmu_state.page_table = MMU_alloc_frame();
   if (!mmu_state.page_table)
      return -1;

   PT_init(map);
   PT_page_table_init(mmu_state.page_table);
   PT_change(mmu_state.page_table);

   return 0;
}

void *MMU_alloc_frame()
{
   void *ret;

   if (mmu_state.free_segment_head) {
      ret = mmu_state.free_segment_head->s.base;
      mmu_state.free_segment_head->s.base += PAGE_SIZE;
      mmu_state.free_segment_head->s.len -= PAGE_SIZE;
      if (!mmu_state.free_segment_head->s.len)
         mmu_state.free_segment_head = mmu_state.free_segment_head->next;
      return ret;
   }

   if (!mmu_state.free_page_head)
      return NULL;

   ret = mmu_state.free_page_head;
   mmu_state.free_page_head = *((void **)mmu_state.free_page_head);
   return ret;
}

void MMU_free_frame(void *addr)
{
   if (!mmu_state.free_page_head) {
      mmu_state.free_page_head = addr;
      mmu_state.free_page_tail = addr;
   } else {
      *((void **)mmu_state.free_page_tail) = addr;
      mmu_state.free_page_tail = addr;
   }
   *((void **)addr) = NULL;
}

void *MMU_alloc_page()
{
   void *ret;

   IRQ_disable();

   if (mmu_state.next_kernel_heap_vaddr == (void *)MMAP_KERNEL_HEAP_END) {
      klog(KLOG_LEVEL_EMERG, "out of kernel heap virtual addresses");
      return NULL;
   }

   ret = mmu_state.next_kernel_heap_vaddr;
   mmu_state.next_kernel_heap_vaddr += PAGE_SIZE;
   PT_demand_allocate(ret);

   IRQ_enable();
   return ret;
}

void *MMU_alloc_pages(int num_pages)
{
   void *first_page;
   int i;

   if (num_pages <= 0)
      return NULL;

   IRQ_disable();

   first_page = MMU_alloc_page();
   if (!first_page)
      return NULL;

   for (i = 0; i < num_pages-1; i++)
      if (!MMU_alloc_page())
         return NULL;

   IRQ_enable();
   return first_page;
}

void MMU_free_page(void *vaddr)
{
   void *addr = PT_addr_virt_to_phys(vaddr);

   if (!addr || (uint64_t)addr % PAGE_SIZE) {
      klog(KLOG_LEVEL_WARN, "attempting to free invalid address");
      return;
   }

   MMU_free_frame(addr);
}

void MMU_free_pages(void *rvaddr, int num_pages)
{
   uint8_t *vaddr = rvaddr;
   void *addr;
   int i;

   for (i = 0; i < num_pages; i++) {
      addr = PT_addr_virt_to_phys(vaddr + (PAGE_SIZE*i));
      if (!addr || ptr_to_int(addr) % PAGE_SIZE) {
         klog(KLOG_LEVEL_WARN, "attempting to free invalid address");
         return;
      }
      MMU_free_frame(addr);
   }
}

void *MMU_alloc_stack()
{
   void *ret;
   int i;

   IRQ_disable();

   if (mmu_state.next_kernel_thread_stack_vaddr == (void *)MMAP_KERNEL_HEAP_START) {
      klog(KLOG_LEVEL_EMERG, "out of kernel stacks virtual addresses");
      return NULL;
   }

   ret = mmu_state.next_kernel_thread_stack_vaddr;
   for (i = 0; i < KERNEL_THREAD_STACK_SIZE_PAGES; i++) {
      PT_demand_allocate(mmu_state.next_kernel_thread_stack_vaddr);
      mmu_state.next_kernel_thread_stack_vaddr -= PAGE_SIZE;
   }

   IRQ_enable();
   return ret;
}

void MMU_free_stack(void *rvaddr)
{
   uint8_t *vaddr = rvaddr;
   void *addr;
   int i;

   vaddr -= (PAGE_SIZE-8);
   for (i = 0; i < KERNEL_THREAD_STACK_SIZE_PAGES; i++) {
      addr = PT_addr_virt_to_phys(vaddr - (PAGE_SIZE*i));
      if (!addr || ptr_to_int(addr) % PAGE_SIZE) {
         klog(KLOG_LEVEL_WARN, "attempting to free invalid address");
         return;
      }
      MMU_free_frame(addr);
   }
}