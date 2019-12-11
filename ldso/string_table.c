#include <link.h>
#include <elf.h>
#include "stdlib.h"
#include <string.h>
#include "stdio.h"
#include "readelf/header_info.h"

static char *table = NULL;
static void *dynamic = NULL;
static void *dynsim = NULL;
static void *strtab = NULL;
static int dynsim_size = 0;

void *get_section_header()
{
    void *header = get_elf_header();
    ElfW(Off) *tmp = get_section_table_offset();
    return (char *)header + *tmp;
}

char *get_string_table()
{
    if (table == NULL)
    {
        void *section_head = get_section_header();
        ElfW(Ehdr) *elf = get_elf_header();
        void *cast = (char *)section_head + elf->e_shstrndx * get_section_entry_size();
        ElfW(Shdr) *head = cast;
        char *tmp = (void *)((char *)elf + head->sh_offset);
        table = tmp;
    }
    return table;
}

void set_str_tab()
{
    uint16_t size = get_section_table_size();
    ElfW(Shdr) *head = get_section_header();
    for (uint16_t i = 0; i < size; i++)
    {
        char *dyn = ".dynstr";
        int eq = 0;
        for (int i = 0; i < 7; i++)
            if (dyn[i] != (table + head->sh_name)[i])
                eq = 1;
        if (eq == 0)
            break;
        char *next = (void *)head;
        next += get_section_entry_size();
        head = (void *)next;
    }
    strtab = (void *)((char *)get_elf_header() + head->sh_offset);
}
void *get_str_tab()
{
    return strtab;
}

void *get_dynamic_section()
{
    if (!dynamic)
    {
        get_string_table();
        ElfW(Shdr) *head = get_section_header();
        set_str_tab();
        ElfW(Ehdr) *elf = get_elf_header();
        uint16_t size = get_section_table_size();
        for (uint16_t i = 0; i < size; i++)
        {
            char *dyn = ".dynamic";
            int eq = 0;
            for (int i = 0; i < 9; i++)
                if (dyn[i] != (table + head->sh_name)[i])
                    eq = 1;
            if (eq == 0)
                break;
            char *next = (void *)head;
            next += get_section_entry_size();
            head = (void *)next;
        }
        dynamic = (char *)elf + head->sh_offset;
    }
    return dynamic;
}

void *get_dynsim_section()
{
    if (!dynsim)
    {
        get_string_table();
        ElfW(Shdr) *head = get_section_header();
        ElfW(Ehdr) *elf = get_elf_header();
        uint16_t size = get_section_table_size();
        for (uint16_t i = 0; i < size; i++)
        {
            if (!strcmp(table + head->sh_name, ".dynsym"))
                break;
            char *next = (void *)head;
            next += get_section_entry_size();
            head = (void *)next;
        }
        dynsim_size = head->sh_size;
        dynsim = (char *)elf + head->sh_offset;
    }
    return dynsim;
}

int get_dynamic_size(ElfW(Dyn) *dynamic)
{
    int i = 0;
    while (dynamic->d_tag != DT_NULL)
        if (dynamic++->d_tag == DT_NEEDED)
            i++;
    return i;
}

void get_needed_entry(ElfW(Dyn) *dynamic, struct link_map *map)
{
    while (dynamic->d_tag != DT_NULL)
    {
        if (dynamic->d_tag == DT_NEEDED)
        {
            char *name = ((char *)strtab + dynamic->d_un.d_val);
            printf("%s\n", name);
            //map_add(map, dynamic, name, 0);
        }
        dynamic++;
    }
}


int get_dynsim_size()
{
    return dynsim_size;
}
