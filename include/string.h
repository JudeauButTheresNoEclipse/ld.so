#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);

int strncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);

void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, void *src, size_t n);

char *strchrnul(const char *s, int c);

#endif /* !STRING_H */
