#include <stdio.h>
#include <link.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>

#include "include/print_headers.h"
#include "include/header_info.h"

static void print_type(uint32_t sh_type)
{
    struct dictionary_entry
    {
        uint32_t type;
        char *msg;
    } type_list[]
    = { { SHT_NULL, "NULL" },         { SHT_PROGBITS, "PROGBITS" },
        { SHT_SYMTAB, "SYMTAB" },     { SHT_STRTAB, "STRTAB" },
        { SHT_RELA, "RELA" },         { SHT_HASH, "HASH" },
        { SHT_DYNAMIC, "DYNAMIC" },   { SHT_NOTE, "NOTE" },
        { SHT_NOBITS, "NOBITS" },     { SHT_REL, "REL" },
        { SHT_SHLIB, "SHLIB" },       { SHT_DYNSYM, "DYNSYM" },
        { SHT_LOPROC, "LOPROC" },     { SHT_HIPROC, "HIPROC" },
        { SHT_LOUSER, "SHT_LOUSER" }, { SHT_HIUSER, "HIUSER" } };
    size_t size = sizeof(type_list) / sizeof(type_list[0]);
    for (size_t i = 0; i < size; i++)
        if (sh_type == type_list[i].type)
            printf("%s\t", type_list[i].msg);
}

static void print_flags(void *section_head)
{
    ElfW(Shdr) *head = section_head;
    if (get_elf_class() == ELFCLASS64)
    {
        printf("%016lx ", head->sh_addr);
        printf("0x%016lx\n      ", head->sh_offset);
        printf("%016lx ", head->sh_size);
        printf("%016lx ", head->sh_entsize);
        if (head->sh_flags & SHF_WRITE)
            printf("%c", 'W');
        if (head->sh_flags & SHF_ALLOC)
            printf("%c", 'A');
        if (head->sh_flags & SHF_EXECINSTR)
            printf("%c", 'X');
        if (head->sh_flags & SHF_MASKPROC)
            printf("MS");
        printf("\t%d\t", head->sh_link);
        printf("%d\t", head->sh_info);
        printf("%ld\n", head->sh_addralign);
    }
    if (get_elf_class() == ELFCLASS32)
    {
        printf("%016x ", (uint32_t)head->sh_addr);
        printf("0x%016x\n      ", (uint32_t)head->sh_offset);
        printf("%016x ", (uint32_t)head->sh_size);
        printf("%016x ", (uint32_t)head->sh_entsize);
        if (head->sh_flags & SHF_WRITE)
            printf("%c", 'W');
        if (head->sh_flags & SHF_ALLOC)
            printf("%c", 'A');
        if (head->sh_flags & SHF_EXECINSTR)
            printf("%c", 'X');
        if (head->sh_flags & SHF_MASKPROC)
            printf("MS");
        printf("\t%d\t", head->sh_link);
        printf("%d\t", head->sh_info);
        printf("%d\n", (uint32_t)head->sh_addralign);
    }
}

static void print_section_header(void *section_head, uint16_t i)
{
    ElfW(Shdr) *head = section_head;
    char *table = get_string_table();
    printf(" [%2d] ", i);
    printf("%s\t\t", table + head->sh_name);
    print_type(head->sh_type);
    print_flags(head);
}

void print_sections_headers()
{
    ElfW(Shdr) *head = get_section_header();
    uint16_t size = get_section_table_size();
    if (get_elf_class() == ELFCLASS64)
        printf("There are %d section headers, starting at offset 0x%lx\n\n",
               size, *(uint64_t *)get_program_table_offset());
    if (get_elf_class() == ELFCLASS32)
        printf("There are %d section headers, starting at offset 0x%x\n\n",
               size, *(uint32_t *)get_program_table_offset());
    printf("Section Headers:\n");
    printf(" [Nr] Name           Type             Address           Offset\n");
    printf("      Size           EntSize          Flags  Link  Info  Align\n");

    for (uint16_t i = 0; i < size; i++)
    {
        print_section_header(head, i);
        char *next = (void *)head;
        next += get_section_entry_size();
        head = (void *)next;
    }
}
