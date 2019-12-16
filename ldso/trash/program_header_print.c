#include <stdio.h>
#include <elf.h>
#include <link.h>
#include "readelf/header_info.h"

static void print_type(uint32_t p_type)
{
    if (p_type == PT_NULL)
        printf(" NULL\t\t");
    if (p_type == PT_LOAD)
        printf(" LOAD\t\t");
    if (p_type == PT_DYNAMIC)
        printf(" DYNAMIC\t");
    if (p_type == PT_INTERP)
        printf(" INTERP\t\t");
    if (p_type == PT_NOTE)
        printf(" NOTE\t\t");
    if (p_type == PT_SHLIB)
        printf(" SHLIB\t\t");
    if (p_type == PT_PHDR)
        printf(" PHDR\t\t");
    if (p_type == PT_LOPROC)
        printf(" LOPROC\t\t");
    if (p_type == PT_GNU_STACK)
        printf(" GNU_STACK\t");
    if (p_type == PT_GNU_EH_FRAME)
        printf(" GNU_EH_FRAME\t");
    if (p_type == PT_GNU_RELRO)
        printf(" GNU_RELRO\t");
}

static void print_flags(uint32_t flags)
{
    if (flags & PF_X)
        printf("X ");
    if (flags & PF_W)
        printf("W ");
    if (flags & PF_R)
        printf("R ");
}

static void print_program_header(void *program_header)
{
    ElfW(Phdr) *head = program_header;
    print_type(head->p_type);
    if (get_elf_class() == ELFCLASS64)
    {
        printf("0x%016lx ", (uint64_t)head->p_offset);
        printf("0x%016lx ", (uint64_t)head->p_vaddr);
        printf("0x%016lx\n", (uint64_t)head->p_paddr);
    }
    if (get_elf_class() == ELFCLASS32)
    {
        printf("0x%016x ", (uint32_t)head->p_offset);
        printf("0x%016x ", (uint32_t)head->p_vaddr);
        printf("0x%016x\n", (uint32_t)head->p_paddr);
    }
    printf("\t\t0x%016lx ", head->p_filesz);
    printf("0x%016lx ", head->p_memsz);
    print_flags(head->p_flags);
    printf("0x%lx\n", head->p_align);
}

void print_programs_headers()
{
    ElfW(Phdr) *head = get_program_headers();
    uint16_t size = get_program_table_size();
    if (get_elf_class() == ELFCLASS64)
        printf("There are %d program headers, starting at offset %ld\n\n", size,
               *(uint64_t *)get_program_table_offset());
    if (get_elf_class() == ELFCLASS32)
        printf("There are %d program headers, starting at offset %d\n\n", size,
               *(uint32_t *)get_program_table_offset());
    printf("Program Headers:\n");
    printf(" Type           Offset             VirtAddr           PhysAddr\n");
    printf(
    "                FileSiz            MemSiz             Flags  Align\n");

    for (uint16_t i = 0; i < size; i++)
    {
        print_program_header(head);
        char *next = (void *)head;
        next += get_program_entry_size();
        head = (void *)next;
    }
}
