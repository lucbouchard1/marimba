#include <stdint.h>
#include "types.h"
#include "mmu.h"
#include "printk.h"
#include "string.h"
#include "utils.h"

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
};

static struct MMUState mmu_state;

/**
 * Converts the KernelSection entries in the SystemMMap into a
 * page-aligned linked list of used memory regions. Returns
 * head pointer to the linked list.
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
   int i;

   /* Copy RAM MMap into free_segments array */
   mmu->free_segment_head = mmu->free_segments;
   for (i = 0, prev = NULL; i < map->num_mmap; i++) {
      curr = &mmu->free_segments[i];
      curr->s.base = map->avail_ram[i].base;
      curr->s.len = map->avail_ram[i].length;
      curr->s.end = curr->s.base + curr->s.len;
      curr->next = NULL;
      if (prev)
         prev->next = curr;
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
}

int MMU_init(struct SystemMMap *map)
{
   struct SegmentList excluded[MAX_EXCLUDE_SEGMENTS];
   struct SegmentList *ex_head, *curr;

   memset(&mmu_state, 0, sizeof(struct MMUState));

   if (map->num_kernel_sects > MAX_EXCLUDE_SEGMENTS) {
      printk("error: too many kernel segments for MMU\n");
      return -1;
   }

   ex_head = mmu_compute_excluded(map, excluded);

   mmu_compute_free_segments(&mmu_state, map, ex_head);

   printk("\n Excluded Sections:\n");
   for (curr = ex_head; curr; curr = curr->next)
      printk("Base: %p   End: %p   Len: 0x%lx\n", curr->s.base, curr->s.end, curr->s.len);

   printk("\n Free Sections:\n");
   for (curr = mmu_state.free_segment_head; curr; curr = curr->next)
      printk("Base: %p   End: %p   Len: 0x%lx\n", curr->s.base, curr->s.end, curr->s.len);

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