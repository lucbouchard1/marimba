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

int memcmp(const void *m1, const void *m2, size_t n)
{
   size_t curr;

   for (curr = 0; curr < n; curr++)
      if (((char *)m1)[curr] != ((char *)m2)[curr])
         return ((char *)m1)[curr] - ((char *)m2)[curr];
   return 0;
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

int strcmp(const char *s1, const char *s2)
{
   size_t curr;

   for (; *s1 && *s2; s1++, s2++)
      if (*s1 != *s2)
         return *s1 - *s2;

   if (*s1 || *s2)
      return *s1 - *s2;

   return 0;
}