#include "include/utility.h"
#include "string.h"
#include "stdlib.h"

char *strdup(char *s)
{
    int len = strlen(s);
    char *res = calloc(len, 2);
    res = memcpy(res, s, len);
    return res;
}
