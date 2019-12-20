#include <elf.h>
#include <link.h>
#include "include/header_info.h"

void *get_program_headers()
{
    void *header = get_elf_header();
    ElfW(Off) *tmp = get_program_table_offset();
    return (char *)header + *tmp;
}
