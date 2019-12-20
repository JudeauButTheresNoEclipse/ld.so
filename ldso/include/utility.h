#ifndef UTILITY_H
#define UTILITY_H

#include <stddef.h>

char *strdup(char *s);
int xopen(char *filename, int flags);
int xread(int fd, void *buff, size_t count);
size_t xlseek(int fildes, size_t offset, int whence);

void *xmalloc(size_t size);

#endif /* !UTILITY_H */
