#include "libdl.h"
#include "include/loader.h"
#include "include/elf_manipulation.h"
#include "include/dependency.h"
#include "include/utility.h"
#include "include/relocations.h"
#include "include/symbol_resolution.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "unistd.h"

#include <elf.h>
#include <link.h>

struct link_map *full_map;
#define PAGE_SIZE 4096


extern void *_dlopen(char *filename, int flags)
{   
    struct link_map *map = full_map;
    if (!filename)
        return map;
    filename = get_lib_absolute_path(filename, map->l_name); 
    elf_ehdr *elf = get_elf_ehdr(filename);
    elf_phdr *phdr = get_program_header(elf, filename);
    struct link_map *new = malloc(sizeof(struct link_map));
    while (map->l_next)
        map = map->l_next;
    new->l_next = NULL;
    new->l_prev = map;
    new->l_name = strdup(filename);
    new->l_addr = map->l_prev->l_addr + 6 * PAGE_SIZE;
    if (!(flags & RTLD_NOLOAD))
        load_program(phdr, elf, new, NULL);
    else
        return new;

    if (flags & RTLD_LAZY)
        resolve_relocations(new, map, 1);
    else if (flags & RTLD_NOW)
        resolve_relocations(new, map, 1);
    else
    {
        puts("flag not set, RTLD_LAZY or RTLD_NOW needed");
        _exit(1);
    }

    if (flags & RTLD_GLOBAL)
        map->l_next = new;
    return new;
}

extern void *_dlsym(void *handle, char *symbol)
{
    struct link_map *map = handle;
    elf_addr *res = (elf_addr *)gnu_hash_lookup(map, symbol);
    return (void*)res;
}

int _dlinfo(void *handle, int request, void *info)
{
     struct link_map *map = handle;
     if (request == RTLD_DI_ORIGIN)
         memcpy(info, map->l_name, strlen(map->l_name));
     else if (request == RTLD_DI_LINKMAP)
         info = map;
     else
         return 1;
     return 0;
}
int _dlclose(void *handle)
{
    struct link_map *map = handle;
    elf_ehdr *elf = get_elf_ehdr(map->l_name);
    elf_phdr *phdr = get_program_header(elf, map->l_name);
    elf_phdr *cpy = phdr;
    elf_addr base = 0;
    while (phdr->p_type != PT_LOAD)
    {
        if (phdr->p_type == PT_NULL)
            return 1;
        phdr++;
    }
    base = phdr->p_vaddr;
    while (phdr->p_type == PT_LOAD)
        phdr++;
    int m = munmap((void *)base, phdr->p_vaddr + phdr->p_filesz - base);
    if (m == -1)
    {
        printf("could not close %s\n", map->l_name);
        return 1;
    }
    if (map->l_prev)
        map->l_prev->l_next = map->l_next;
    if (map->l_next)
        map->l_next->l_prev = map->l_prev;
    free(elf);
    free(cpy);
    return 0;
}

extern int _dladdr(void *addr, Dl_info *info)
{
    struct link_map *map = full_map;
    while (map)
    {
        if ((elf_addr)addr >= map->l_addr && addr <= (void *)(map->l_ld + (elf_addr)((PAGE_SIZE))))
            break;
        else
            map = map->l_next;
    }
    if (!map || !map->l_name)
    {
        info->dli_sname = NULL;
        info->dli_saddr = NULL;
        return 0;
    }
    else
    {
        info->dli_sname = strdup(map->l_name);
        info->dli_saddr = (void *)map->l_addr;
        elf_ehdr *elf = get_elf_ehdr(map->l_name);
        elf_sym *symtab = get_section(elf, map->l_name, ".dynsym");
        if (!symtab)
            return 0;
        char *dynstr = (char *)get_section(elf, map->l_name, ".dynstr");
        if (!dynstr)
        {
            free(symtab);
            return 0;
        }
        int size = get_section_size(elf, map->l_name, ".dynsym");
        for (int i = 0; i < size; i++)
        {
            elf_addr down = symtab->st_value + map->l_addr;
            elf_addr up = symtab->st_value + map->l_addr + symtab->st_size;
            if ((elf_addr)addr >= down && (elf_addr)addr <= up)
            {
                info->dli_fbase = (void *)(symtab->st_value);
                info->dli_fname = strdup(dynstr + symtab->st_name);
                break;
            }
            symtab++;
        }
        free(symtab);
        free(dynstr);
        free(elf);
    }
    return 1;
}
