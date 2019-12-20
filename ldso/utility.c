#include "include/utility.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"

char *strdup(char *s)
{
    int len = strlen(s);
    char *res = calloc(len, 2);
    res = memcpy(res, s, len);
    return res;
}

int xopen(char *filename, int flags)
{
    int err = open(filename, flags);
    if (err < 0)
    {
        printf("could not open %\n", filename);
        _exit(1);
    }
    return err;
}

int xread(int fd, void *buff, size_t count)
{
    int err = read(fd, buff, count);
    if (err < 0)
    {
        printf("could not read in %lx\n", buff);
        _exit(1);
    }
    return err;
}

size_t xlseek(int fildes, size_t offset, int whence)
{
    int err = lseek(fildes, offset, whence);
    if (err < 0)
    {
        printf("lseek failed on fd %d\n", fildes);
        _exit(1);
    }
    return err;
}

void *xmalloc(size_t size)
{
    void *res = malloc(size);
    if (!res)
    {
        printf("Out of memory");
        _exit(1);
    }
    return res;
}
