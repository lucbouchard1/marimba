#include "types.h"
#include "mmu.h"
#include "printk.h"

#define MAX_FREE_SEGMENTS 32
#define MAX_EXCLUDE_SEGMENTS 128

struct Segment {
   uint8_t *base;
   size_t len;
};

struct SegmentList {
   struct SegmentList *next;
   struct Segment s;
};

struct MMUState {
   struct Segment free_segments[MAX_EXCLUDE_SEGMENTS];
};

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
   int i;

   if (map->num_kernel_sects > MAX_EXCLUDE_SEGMENTS) {
      printk("error: too many kernel segments for MMU\n");
      return -1;
   }

   /* Copy kernel segments into excluded segments array */
   ex_head = &excluded[0];
   prev = NULL;
   for (i = 0; i < map->num_kernel_sects; i++) {
      excluded[i].s.base = map->kernel_sects[i].base;
      excluded[i].s.len = map->kernel_sects[i].length;
      excluded[i].next = NULL;
      if (prev)
         prev->next = &excluded[i];
      prev = &excluded[i];
   }

   /* Round excluded sections to page boundries */
   ROUND down the base address, round up the length. Shouldnt cause any issues
   for (curr = ex_head->next, prev = ex_head; curr; prev = curr, curr = curr->next) {
      if (prev->s.base + prev->s.len < curr->s.base)
         continue;

      prev->s.len = max(prev->s.len, (curr->s.base + curr->s.len) - prev->s.base);
      prev->next = curr->next;
      curr = prev;
   }

   /* Reduce overlapping kernel segments into contiguous segments */
   for (curr = ex_head->next, prev = ex_head; curr; prev = curr, curr = curr->next) {
      if (prev->s.base + prev->s.len < curr->s.base)
         continue;

      prev->s.len = max(prev->s.len, (curr->s.base + curr->s.len) - prev->s.base);
      prev->next = curr->next;
      curr = prev;
   }

   for (curr = ex_head; curr; curr = curr->next)
      printk("Base: %p   Len: 0x%lx\n", curr->s.base, curr->s.len);

   return 0;
}