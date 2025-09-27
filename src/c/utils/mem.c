#include "mem.h"

void *memset(void *dest, int val, unsigned int len) {
    unsigned char *ptr = dest;
    for (unsigned int i = 0; i < len; i++)
        ptr[i] = (unsigned char)val;
    return dest;
}

unsigned int strlen(const char *str) {
    unsigned int len = 0;
    while (str[len] != '\0') len++;
    return len;
}