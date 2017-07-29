/*
 * arch/arm/boot/compressed/string.c
 *
 * Small subset of simple string routines
 */

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

void *memmove(void *__dest, __const void *__src, size_t count)
{
	unsigned char *d = __dest;
	const unsigned char *s = __src;

	if (__dest == __src)
		return __dest;

	if (__dest < __src)
		return memcpy(__dest, __src, count);

	while (count--)
		d[count] = s[count];
	return __dest;
}

size_t strlen(const char *s)
{
	const char *sc = s;

	while (*sc != '\0')
		sc++;
	return sc - s;
}

size_t strnlen(const char *s, size_t maxlen)
{
        const char *es = s;
        while (*es && maxlen) {
                es++;
                maxlen--;
        }

        return (es - s);
}

int memcmp(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1 = cs, *su2 = ct, *end = su1 + count;
	int res = 0;

	while (su1 < end) {
		res = *su1++ - *su2++;
		if (res)
			break;
	}
	return res;
}

int strcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;
	int res = 0;

	do {
		c1 = *cs++;
		c2 = *ct++;
		res = c1 - c2;
		if (res)
			break;
	} while (c1);
	return res;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0)
        return 0;

    while (n-- != 0 && *s1 == *s2)
    {
        if (n == 0 || *s1 == '\0')
            break;
        s1++;
        s2++;
    }

    return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}

void *memchr(const void *s, int c, size_t count)
{
	const unsigned char *p = s;

	while (count--)
		if ((unsigned char)c == *p++)
			return (void *)(p - 1);
	return NULL;
}

char *strchr(const char *s, int c)
{
	while (*s != (char)c)
		if (*s++ == '\0')
			return NULL;
	return (char *)s;
}

//#undef memset

void *memset(void *s, int c, size_t count)
{
	char *xs = s;
	while (count--)
		*xs++ = c;
	return s;
}

void __memzero(void *s, size_t count)
{
	memset(s, 0, count);
}

// ------------------------------------------

#define TOLOWER(x) ((x) | 0x20)
#define isxdigit(c)    (('0' <= (c) && (c) <= '9') \
                     || ('a' <= (c) && (c) <= 'f') \
                     || ('A' <= (c) && (c) <= 'F'))

#define isdigit(c)    ('0' <= (c) && (c) <= '9')

unsigned long strtoul(const char *cp, char **endp, unsigned int base)
{
    unsigned long result = 0;
    unsigned long value;

    if (!base)
    {
        base = 10;
        if (*cp == '0')
        {
            base = 8;
            cp++;
            if ((TOLOWER(*cp) == 'x') && isxdigit(cp[1]))
            {
                cp++;
                base = 16;
            }
        }
    }
    else if (base == 16)
    {
        if (cp[0] == '0' && TOLOWER(cp[1]) == 'x')
        {
            cp += 2;
        }
    }

    while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : TOLOWER(*cp)-'a'+10) < base)
    {
        result = result*base + value;
        cp++;
    }

    if (endp)
    {
        *endp = (char *)cp;
    }

    return result;
}

long strtol(const char *cp, char **endp, unsigned int base)
{
    if(*cp=='-')
    {
        return -strtoul(cp+1,endp,base);
    }
    else
    {
        return strtoul(cp,endp,base);
    }
}

int atoi(const char* nptr)
{
    return strtol(nptr, (char **)NULL, 10);
}

// ------------------------------------------

