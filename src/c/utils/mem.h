#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

void *memset(void *dest, int val, size_t len);
size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strchr(const char *s, int c);
char *strcat(char *dest, const char *src);

#endif