#include "string.h"

void *mmemset(void *dest, int c, size_t n)
{
   char *idest = dest;

   while ((idest - (char *)dest) < n)
      *(idest++) = (char)c;

   return dest;
}