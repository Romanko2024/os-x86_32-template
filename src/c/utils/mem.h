#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

void *memset(void *dest, int val, size_t len);
size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);

#endif