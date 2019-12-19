#include "stdio.h"
//#include "stdlib.h"
//#include "unistd.h"
int main(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
	{
		puts(argv[i]);
		//printf("%d\n", i);
        //printf("%s\n", argv[i]);
	}
	//int *qqc = malloc(sizeof(int));
	//gettimeofday(10, 0);
    return 0;
}
