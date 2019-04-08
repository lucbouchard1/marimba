#ifndef __STRING_H__
#define __STRING_H__

#include "types.h"

void *memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
size_t strlen(const char *s);
//char *mstrcpy(char *dest, const char *src);
//int mstrcmp(const char *s1, const char *s2);
//const char *mstrchr(const char *s, int c);
// TODO: char *strdup(const char *s);

#endif