#include "link_map.h"
#include "stdlib.h"
#include "string.h"
#include "readelf/header_info.h"
#include "readelf/read_elf_file.h"
#include "ldso_include/ldso.h"
#include "stdio.h"
#include "unistd.h"


#include <linux/mman.h>
#include <elf.h>
#include <link.h>


static void load_dso(struct link_map *map, int filedes);




struct link_map *link_map_add(struct link_map *map, char *name, ElfW(auxv_t) *auxv)
{
    for (struct link_map *next = map; next; next = next->l_next)
        if (!strcmp(name, next->l_name))
            return NULL;
    
    int size = 0;
    int filedes = open(name, O_RDWR);
    void *file;
    if (strcmp(name, "linux-vdso.so.1"))
    {
        file = read_elf_file(name, &size);
    }
    else
        file = (void *)get_auxv_entry(auxv, AT_SYSINFO_EHDR)->a_un.a_val;

    struct link_map *new = malloc(sizeof(struct link_map));
    new->l_addr = (ElfW(Addr))file; 
    new->l_name = name;
    new->l_ld = get_dynamic_section();
    new->l_next = NULL; 
    if (strcmp(name, "linux-vdso.so.1"))
        load_dso(new, filedes);

    if (!map)
    {
        new->l_prev = NULL;
        return new;
    }
    
    struct link_map *first = map;
    while (map->l_next)
        map = map->l_next;
    map->l_next = new;
    new->l_prev = map;

    return first;
}

static void load_dso(struct link_map *map, int filedes, ElfW(auxv_t) *auxv)
{
    ElfW(Ehdr) *elf = (void *)map->l_addr;
    ElfW(Phdr) *program_header = (void *)(map->l_addr + elf->e_phoff);
    int nb_header = elf->e_phnum;
    int nb_load = 0;
    for (int i = 0; i < nb_header; i++)
        if ((program_header + i)->p_type == PT_LOAD)
            nb_load++;
    if (nb_load == 0)
        return;
    while (program_header->p_type != PT_LOAD)
        program_header++;
    int prot = PROT_READ | PROT_WRITE | PROT_EXEC;
    int flags = MAP_PRIVATE;
    for (int i = 0; i < nb_load; i++)
    {
        mmap((void *)program_header->p_vaddr, program_header->p_filesz, prot, flags, filedes, program_header->p_offset);
        program_header++;
    }
}
