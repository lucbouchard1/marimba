#include <stdint.h>
#include "types.h"
#include "mmu.h"
#include "printk.h"
#include "string.h"

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
   struct SegmentList *free_head;
};

static struct MMUState mmu_state;

int max(int i1, int i2)
{
   if (i1 > i2)
      return i1;
   return i2;
}

int MMU_init(struct SystemMMap *map)
{
   struct SegmentList excluded[MAX_EXCLUDE_SEGMENTS];
   struct SegmentList *ex_head, *prev, *curr;
   struct SegmentList *fcurr, *ecurr;
   uint8_t *new_base;
   int i;

   memset(&mmu_state, 0, sizeof(struct MMUState));

   if (map->num_kernel_sects > MAX_EXCLUDE_SEGMENTS) {
      printk("error: too many kernel segments for MMU\n");
      return -1;
   }

   /* Copy kernel segments into excluded segments array */
   ex_head = &excluded[0];
   for (i = 0, prev = 0; i < map->num_kernel_sects; i++) {
      excluded[i].s.base = map->kernel_sects[i].base;
      excluded[i].s.len = map->kernel_sects[i].length;
      excluded[i].s.end = excluded[i].s.base + excluded[i].s.len;
      excluded[i].next = 0;
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

   /* Copy RAM MMap into free_segments array */
   mmu_state.free_head = mmu_state.free_segments;
   for (i = 0, prev = 0; i < map->num_mmap; i++) {
      curr = &mmu_state.free_segments[i];
      curr->s.base = map->avail_ram[i].base;
      curr->s.len = map->avail_ram[i].length;
      curr->s.end = curr->s.base + curr->s.len;
      if (prev)
         prev->next = curr;
      prev = curr;
   }

   /* Remove excluded sections from free segments array */
   prev = 0;
   for (fcurr = mmu_state.free_head, ecurr = ex_head; 
         fcurr && ecurr; ) {
      if (fcurr->s.base < ecurr->s.base && fcurr->s.end > ecurr->s.base) {
         fcurr->s.len = ecurr->s.base - fcurr->s.base;
         if (fcurr->s.end > ecurr->s.end) {
            /* Insert new free entry */
            mmu_state.free_segments[i].s.base = ecurr->s.end;
            mmu_state.free_segments[i].s.len = fcurr->s.end - ecurr->s.end;
            mmu_state.free_segments[i].next = fcurr->next;
            fcurr->next = &mmu_state.free_segments[i];
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
               mmu_state.free_head = fcurr->next;
         } else {
            fcurr->s.len = fcurr->s.end - ecurr->s.end;
            fcurr->s.base = ecurr->s.end;
            ecurr = ecurr->next;
         }
      } else {
         fcurr = fcurr->next;
      }
   }

   printk("\n Excluded Sections:\n");
   for (curr = ex_head; curr; curr = curr->next)
      printk("Base: %p   End: %p   Len: 0x%lx\n", curr->s.base, curr->s.end, curr->s.len);

   printk("\n Free Sections:\n");
   for (curr = mmu_state.free_head; curr; curr = curr->next)
      printk("Base: %p   End: %p   Len: 0x%lx\n", curr->s.base, curr->s.end, curr->s.len);

   return 0;
}