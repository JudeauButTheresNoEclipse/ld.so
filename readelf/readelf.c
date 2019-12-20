#include <link.h>
#include <elf.h>
#include <stdio.h>
#include "include/header_info.h"
#include "include/print_headers.h"
#include "include/read_elf_file.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        char *error = "usage: [dummy-readelf $binary]";
        puts(error);
        return 1;
    }

    int size= 0;
    char *file = read_elf_file(argv[1], &size);

    print_elf_header();
    print_programs_headers();
    print_sections_headers();
    print_dynamic_section();
    print_dynsim_section();

    free_file(file,0 );
    return 0;
}
