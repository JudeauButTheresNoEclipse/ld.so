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
