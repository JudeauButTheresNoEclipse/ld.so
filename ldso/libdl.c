#include "libdl.h"
#include "include/functions.h"
#include "include/elf_manipulation.h"
#include "include/dependency.h"
#include "include/utility.h"
#include "include/relocations.h"
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
    new->l_prev = map;
    new->l_name = strdup(filename);
    new->l_addr = map->l_prev->l_addr + 6 * PAGE_SIZE;
    new->l_ld = 1;
    if (!(flags & RTLD_NOLOAD))
        load_program(phdr, elf, new);
    else
        return new;

    if (flags & RTLD_LAZY)
        resolve_relocations(new, map, 1);
    else if (flags & RTLD_NOW)
        resolve_relocations(new, map, 1);
    else
    {
        puts("you have to say if you want lazy binding");
        _exit(1);
    }

    if (flags & RTLD_GLOBAL)
        map->l_next = new;

    return new;
}

extern void *_dlsym(void *handle, char *symbol)
{
    struct link_map *map = handle;
    elf_addr *res = gnu_hash_lookup(map, symbol);
    map->l_ld = (void *)((char *)(void *)map->l_ld + 1);
    return (void*)res;
}

int dlinfo(void *handle, int request, void *info);
int dlclose(void *handle)
{
    struct link_map *map = handle;
    char *ref = (void *)map->l_ld;
    ref--;
    if (!ref)
    {
        //TODO unload map;
        map->l_prev->l_next = map->l_next;
        map->l_next->l_prev = map->l_prev;
        return 0;
    }
    return (elf_addr)ref;
}
int dladdr(void *addr, Dl_info *info);
