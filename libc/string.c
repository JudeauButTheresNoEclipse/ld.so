#include <stddef.h>

#include "types.h"
#include "stdlib.h"


size_t strlen(const char *s)
{
	size_t n = 0;
	while (*s++) {
		n++;
	}
	return n;
}

size_t strnlen(const char *s, size_t maxlen)
{
	size_t len = 0;
	for (; len < maxlen; len++)
		if (!s[len])
			return len;
	return len;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	const unsigned char *l = (void *)s1;
	const unsigned char *r = (void *)s2;

	if (!n--)
		return 0;

	for (; *l && *r && n && *l == *r ; l++, r++, n--)
		continue;

	return *l - *r;
}

int strcmp(const char *s1, const char *s2)
{
	size_t i = 0;
	for (; s1[i]; i++) {
		if (s1[i] != s2[i]) {
			break;
		}
	}
	return (u8)s1[i] - (u8)s2[i];
}

void *memset(void *s, int c, size_t n)
{
	u8 *b = s;
	for (size_t i = 0; i < n; i++) {
		b[i] = c;
	}
	return s;
}

void *memcpy(void *dest, void *src, size_t n)
{
	u8 *d = dest;
	u8 *s = src;
	for (size_t i = 0; i < n; i++) {
		d[i] = s[i];
	}
	return dest;
}

char *strchrnul(const char *s, int c)
{
	for (; *s; s++) {
		if ((unsigned char)*s == (unsigned char)c)
			break;
	}
	return (char *)s;
}
