#ifndef READ_ELF_FILE_C
#define READ_ELF_FILE_C

#include <link.h>
#include <elf.h>

void *read_elf_file(char *filename, int *size);
int get_file_size(char *filename);
void free_file(char *ptr, int size);

#endif /*!READ_ELF_FILE_C*/
