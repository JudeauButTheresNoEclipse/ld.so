#include <elf.h>
#include <link.h>

void *elf_header = 0;

void *get_elf_header()
{
    return elf_header;
}

void set_elf_header(void *addr)
{
    elf_header = addr;
}

void *get_entry()
{
    ElfW(Ehdr) *head = elf_header;
    return &(head->e_entry);
}

void *get_program_table_offset()
{
    ElfW(Ehdr) *head = elf_header;
    return &(head->e_phoff);
}

uint16_t get_program_entry_size()
{
    ElfW(Ehdr) *head = elf_header;
    return head->e_phentsize;
}

uint16_t get_program_table_size()
{
    ElfW(Ehdr) *head = elf_header;
    return head->e_phnum;
}

void *get_section_table_offset()
{
    ElfW(Ehdr) *head = elf_header;
    return &(head->e_shoff);
}

uint16_t get_section_entry_size()
{
    ElfW(Ehdr) *head = elf_header;
    return head->e_shentsize;
}

uint16_t get_section_table_size()
{
    ElfW(Ehdr) *head = elf_header;
    return head->e_shnum;
}

unsigned char get_elf_class()
{
    ElfW(Ehdr) *head = elf_header;
    return head->e_ident[EI_CLASS];
}

