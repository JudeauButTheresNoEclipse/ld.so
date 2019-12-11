#include <elf.h>
#include <link.h>
#include "readelf/header_info.h"

void *get_program_headers()
{
    void *header = get_elf_header();
    ElfW(Off) *tmp = get_program_table_offset();
    return (char *)header + *tmp;
}
