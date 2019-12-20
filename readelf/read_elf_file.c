#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "include/header_info.h"

#define AT_FDCWD -100
int get_file_size(char *filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

void *read_elf_file(char *filename, int *file_size)
{
    int filedes = open(filename, O_RDONLY);
    if (filedes == -1)
        return MAP_FAILED;
    *file_size = get_file_size(filename);
    if (*file_size == -1)
        return MAP_FAILED;
    void *file = mmap(0, *file_size, PROT_READ | PROT_EXEC | PROT_WRITE, MAP_PRIVATE, filedes, 0);
    if (file == MAP_FAILED)
        return MAP_FAILED;
    set_elf_header(file);
    return file;
}

void free_file(char *ptr, int size)
{
    munmap(ptr, size);
}
