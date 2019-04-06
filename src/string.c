#include "string.h"

void *mmemset(void *dest, int c, size_t n)
{
   char *cur = dest, *end = ((char *)dest + n);

   while (cur != end)
      *(cur++) = (char)c;

   return dest;
}