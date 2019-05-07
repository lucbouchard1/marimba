#include <stddef.h>
#include "utils.h"

void sort(void *arr, size_t num_els, size_t el_len,
      int (*cmp)(void *arg1, void *arg2),
      void (*swp)(void *arg1, void *arg2))
{
   uint8_t *curr, *temp, *min;

   if (!num_els || num_els == 1)
      return;

   /* Selection sort */
   for (curr = arr; curr != ((uint8_t *)arr) + el_len*(num_els-1);
         curr += el_len) {
      for (min = curr, temp = min + el_len;
            temp != ((uint8_t *)arr) + el_len*(num_els);
            temp += el_len) {
         if (cmp(min, temp) < 0)
            min = temp;
      }
      if (min != curr)
         swp(curr, min);
   }
}

int kernel_mmap_cmp(void *rarg1, void *rarg2)
{
   struct KernelSection *arg1 = (struct KernelSection *)rarg1;
   struct KernelSection *arg2 = (struct KernelSection *)rarg2;

   if (arg1->base > arg2->base)
      return -1;
   else if (arg2->base > arg1->base)
      return 1;
   else
      return 0;
}

void kernel_mmap_swp(void *rarg1, void *rarg2)
{
   struct KernelSection *arg1 = (struct KernelSection *)rarg1;
   struct KernelSection *arg2 = (struct KernelSection *)rarg2;
   struct KernelSection temp;

   temp = *arg2;
   *arg2 = *arg1;
   *arg1 = temp;
}

void sort_kernel_section_array(struct KernelSection *arr, size_t len)
{
   sort(arr, len, sizeof(struct KernelSection),
         kernel_mmap_cmp, kernel_mmap_swp);
}

int mmap_entry_cmp(void *rarg1, void *rarg2)
{
   struct KernelSection *arg1 = (struct KernelSection *)rarg1;
   struct KernelSection *arg2 = (struct KernelSection *)rarg2;

   if (arg1->base > arg2->base)
      return -1;
   else if (arg2->base > arg1->base)
      return 1;
   else
      return 0;
}

void mmap_entry_swp(void *rarg1, void *rarg2)
{
   struct KernelSection *arg1 = (struct KernelSection *)rarg1;
   struct KernelSection *arg2 = (struct KernelSection *)rarg2;
   struct KernelSection temp;

   temp = *arg2;
   *arg2 = *arg1;
   *arg1 = temp;
}

void sort_mmap_entry_array(struct MMapEntry *arr, size_t len)
{
   sort(arr, len, sizeof(struct MMapEntry),
         mmap_entry_cmp, mmap_entry_swp);
}