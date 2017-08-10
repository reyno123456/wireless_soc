#include <string.h>

void *memcpy(void *__dest, __const void *__src, size_t __n)
{
    int i = 0;
    unsigned char *d = (unsigned char *)__dest, *s = (unsigned char *)__src;

    for (i = __n >> 3; i > 0; i--) {
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
    }

    if (__n & 1 << 2) {
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
    }

    if (__n & 1 << 1) {
        *d++ = *s++;
        *d++ = *s++;
    }

    if (__n & 1)
        *d++ = *s++;

    return __dest;
}

void *memset(void *s, int c, size_t count)
{
    char *xs = s;
    while (count--)
        *xs++ = c;
    return s;
}

