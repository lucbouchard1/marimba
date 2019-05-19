#include <stdint.h>
#include "types.h"
#include "mmu.h"
#include "printk.h"
#include "string.h"
#include "utils.h"
#include "interrupts.h"
#include "page_table.h"

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
};

static struct MMUState mmu_state;

static void page_fault_handler(int irq, int err, void *arg)
{

}

/**
 * Converts the KernelSection entries in the SystemMMap into a
 * page-aligned linked list of used memory regions.
 * 
 * @retval Head pointer to the linked list.
 */
static struct SegmentList *mmu_compute_excluded(struct SystemMMap *map,
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
      new_base = (void *)((uint64_t)curr->s.base - ((uint64_t)curr->s.base % MMU_PAGE_SIZE));
      curr->s.len += curr->s.base - new_base;
      curr->s.base = new_base;
      /* Round up length to page size */
      if (curr->s.len % MMU_PAGE_SIZE) {
         curr->s.len += MMU_PAGE_SIZE;
         curr->s.len -= (curr->s.len % MMU_PAGE_SIZE);
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
 * free memory regions. Uses the MMap entries in the SystemMMap
 */
static void mmu_compute_free_segments(struct MMUState *mmu, struct SystemMMap *map,
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
      if (new_base % MMU_PAGE_SIZE) {
         new_base += MMU_PAGE_SIZE;
         new_base -= (new_base % MMU_PAGE_SIZE);
      }
      curr->s.len = (uint64_t)curr->s.end - new_base;
      curr->s.base = (void *)new_base;
      /* Round down length to page size */
      curr->s.len = curr->s.len - (curr->s.len % MMU_PAGE_SIZE);
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
      if (mmu->free_segment_head->s.len == MMU_PAGE_SIZE) {
         mmu->free_segment_head = mmu->free_segment_head->next;
      } else {
         mmu->free_segment_head->s.base += MMU_PAGE_SIZE;
         mmu->free_segment_head->s.len -= MMU_PAGE_SIZE;
      }
   }
}

int MMU_init(struct SystemMMap *map)
{
   struct SegmentList excluded[MAX_EXCLUDE_SEGMENTS];
   struct SegmentList *ex_head;

   memset(&mmu_state, 0, sizeof(struct MMUState));

   if (map->num_kernel_sects > MAX_EXCLUDE_SEGMENTS) {
      printk("error: too many kernel segments for MMU\n");
      return -1;
   }

   ex_head = mmu_compute_excluded(map, excluded);
   mmu_compute_free_segments(&mmu_state, map, ex_head);

   mmu_state.page_table = MMU_pf_alloc();
   if (!mmu_state.page_table)
      return -1;

   PT_init(map);
   PT_page_table_init(mmu_state.page_table);
   PT_change(mmu_state.page_table);

   IRQ_set_handler(PAGE_FAULT_IRQ, page_fault_handler, &mmu_state);

   return 0;
}

void *MMU_pf_alloc()
{
   void *ret;

   if (mmu_state.free_segment_head) {
      ret = mmu_state.free_segment_head->s.base;
      mmu_state.free_segment_head->s.base += MMU_PAGE_SIZE;
      mmu_state.free_segment_head->s.len -= MMU_PAGE_SIZE;
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

void MMU_pf_free(void *pf)
{
   if (!mmu_state.free_page_head) {
      mmu_state.free_page_head = pf;
      mmu_state.free_page_tail = pf;
   } else {
      *((void **)mmu_state.free_page_tail) = pf;
      mmu_state.free_page_tail = pf;
   }
   *((void **)pf) = NULL;
}

void MMU_stress_test()
{
   void *page_buff[100], *res = NULL;
   int i, cycle, total_cycles = 10000;
   const char *bit_patt_base = "bit pattern woohoo!";
   int bit_patt_len = strlen(bit_patt_base);
   int bit_patt_off = 20;

   printk("Large Allocation Stress Test:\n");
   for (cycle = 0; cycle <= total_cycles; cycle++) {
      printk("Allocating and freeing 100 pages [%d/%d]", cycle, total_cycles);
      for (i = 0; i < 100; i++)
         page_buff[i] = MMU_pf_alloc();

      for (i = 0; i < 100; i++)
         MMU_pf_free(page_buff[i]);
      printk("\r");
   }
   printk("\nDone.\n");

   printk("Bit Pattern Stress Test:\n");
   printk("   applying bit patterns on all pages...\n");
   for (cycle = 0; 1; cycle++) {
      res = MMU_pf_alloc();
      /* Check if we've gotten through all the pages by looking for the bit pattern */
      if (!memcmp(&((char *)res)[bit_patt_off], bit_patt_base, bit_patt_len))
         break;
      /* Write bit pattern to page */
      memcpy(&((char *)res)[bit_patt_off], bit_patt_base, bit_patt_len);
      ((int *)res)[bit_patt_off + bit_patt_len] = cycle;
      MMU_pf_free(res);
   }
   printk("   applied bit pattern to %d pages...\n", cycle);
   printk("   checking bit patterns on all pages...\n");
   for (i = 0; i < cycle; i++) {
      MMU_pf_free(res);
      if (memcmp(&((char *)res)[bit_patt_off], bit_patt_base, bit_patt_len) ||
               ((int *)res)[bit_patt_off + bit_patt_len] != i)
         printk("Bit Pattern Failure!\n");
      res = MMU_pf_alloc();
   }
   MMU_pf_free(res);
   printk("   checked bit pattern on %d pages...\n", i);
   printk("Done.\n");

   printk("Max Allocation Stress Test:\n");
   res = MMU_pf_alloc();
   for (cycle = 1; res; cycle++)
      res = MMU_pf_alloc();
   printk("Page frame alloc returned NULL after allocating %d pages\n", cycle);
   printk("Done.\n");
}