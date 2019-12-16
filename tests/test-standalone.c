#include "stdio.h"
#include "unistd.h"
int main(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
	{
		printf("%d\n", argc);
        printf("%s\n", argv[i]);
	}
	//gettimeofday(0, 0);
    return 0;
}
