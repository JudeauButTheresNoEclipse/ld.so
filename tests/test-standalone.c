#include "stdio.h"
#include "../ldso/libdl.h"

int main(int argc, char **argv)
{
	void *ptr = (void *)_dlopen("libstring.so", 1);
	int (*strlen)(char *) = _dlsym(ptr, "strlen");

	for (int i = 0; i < argc; i++)
	{
		puts(argv[i]);
		printf("%d\n", i);
	}
   	printf("STRLEN %d\n", strlen("hahaha"));
    
    return 0;
}
