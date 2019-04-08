#include "string.h"

void *memset(void *dest, int c, size_t n)
{
   char *cur = dest, *end = ((char *)dest + n);

   while (cur != end)
      *(cur++) = (char)c;

   return dest;
}

void *memcpy(void *dest, const void *src, size_t n)
{
   char *dcur = dest, *end = ((char *)dest + n);
   const char *scur = src;

   while (dcur != end)
      *(dcur++) = *(scur++);

   return dest;
}

size_t strlen(const char *s)
{
   const char *cur;
   size_t count = 0;

   for (cur = s; *cur; cur++) count++;

   return count;
}

char *strcpy(char *dest, const char *src)
{
   char *dcur = dest;
   const char *scur = src;

   do {
      *(dcur++) = *(scur++);
   } while (*scur);

   return dest;
}