#ifndef __STRING_H__
#define __STRING_H__

#include "types.h"

void *memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *m1, const void *m2, size_t n);
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
// TODO: char *strdup(const char *s);

#endif