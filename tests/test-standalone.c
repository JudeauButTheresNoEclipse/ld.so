#include "stdio.h"
#include "../ldso/libdl.h"

int main(int argc, char **argv)
{
	void *ptr = (void *)_dlopen("libstring.so", 1 | RTLD_GLOBAL);
	int (*strlen)(char *) = _dlsym(ptr, "strlen");

	for (int i = 0; i < argc; i++)
	{
		puts(argv[i]);
		printf("%d\n", i);
	}
   	printf("STRLEN %d\n", strlen("hahaha"));
    Dl_info info = {0, 0, 0, 0};
    _dladdr(strlen, &info);
   	//printf("STRLEN %d\n", strlen("hahaha"));
    printf("%s\n", info.dli_sname);
    //printf("%s\n", info.dli_fname);
    return 0;
}
