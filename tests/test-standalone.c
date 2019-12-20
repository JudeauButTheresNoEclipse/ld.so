#include "stdio.h"
#include "../ldso/libdl.h"
//#include "stdlib.h"
//#include "unistd.h"

int main(int argc, char **argv)
{
	/*void *ptr = (void *)_dlopen("libstring.so", 0);
	int (*strlen)(char *) = _dlsym(ptr, "strlen");
	printf("%lx\n", strlen);
   	printf("STRLEN %d\n", strlen("hahaha"));*/

	for (int i = 0; i < argc; i++)
	{
		puts(argv[i]);
		printf("%d\n", i);
        //printf("%s\n", argv[i]);
	}
	//printf("%d\n", strlen("hahaha"));
	//int *qqc = malloc(sizeof(int));
	//gettimeofday(10, 0);
    return 0;
}
