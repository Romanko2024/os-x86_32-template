#include "mem.h"

void *memset(void *dest, int val, size_t len) {
    uint8_t *ptr = (uint8_t*)dest;
    for (size_t i = 0; i < len; i++) {
        ptr[i] = (uint8_t)val;
    }
    return dest;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') len++;
    return len;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}