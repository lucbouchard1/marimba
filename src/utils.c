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