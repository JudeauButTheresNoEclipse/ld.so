#include <stdbool.h>
#include <elf.h>
#include <link.h>
#include <stddef.h>
#include <sys/auxv.h>
#include <linux/mman.h>
#include <asm-generic/fcntl.h>

#include "string.h"
#include "types.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "ldso_include/display_auxv.h"
#include "readelf/read_elf_file.h"
#include "readelf/header_info.h"
#include "link_map.h"


ElfW(auxv_t) * get_auxv_entry(ElfW(auxv_t) * auxv, u32 type)
{
    for (; auxv->a_type != AT_NULL; auxv++)
        if (auxv->a_type == type)
            return auxv;
    return NULL;
}

ElfW(auxv_t) * find_auxv(char **envp)
{
    while (*envp != NULL)
        envp++;
    envp++;
    return (ElfW(auxv_t) *)envp;
}

static inline void jmp_to_usercode(u64 entry, u64 stack)
{
    asm volatile("mov %[stack], %%rsp\n"
                 "push %[entry]\n"
                 "ret" ::[entry] "r"(entry),
                 [stack] "r"(stack));
}

static void build_link_map_rec(char *filename, struct link_map *map)
{
    int size = 0;
    void *file = read_elf_file(filename, &size);
    if (file == MAP_FAILED)
        return ;
    
    link_map_add(map, filename, file);
    if (!map)
    {
        free_file(file, size);
        return;
    }
    
    ElfW(Dyn) *dynamic = get_dynamic_section();
    char *needed = NULL;
    while ((needed = get_needed_entry(&dynamic)) != NULL)
            build_link_map_rec(needed, map);
    return;
}

static struct link_map *build_link_map(char **envp, void *file)
{
    ElfW(auxv_t) *auxv = find_auxv(envp);
    void *vdso = (void *)get_auxv_entry(auxv, AT_SYSINFO_EHDR)->a_un.a_val;
    set_elf_header(vdso);
    struct link_map *map = link_map_add(NULL, "linux-vdso.so.1", vdso);
    set_elf_header(file);
    ElfW(Dyn) *dynamic = get_dynamic_section();
    
    char *needed = NULL;
    while ((needed = get_needed_entry(&dynamic)) != NULL)
        build_link_map_rec(needed, map);
    build_link_map_rec("ld.so", map);
    return map;
}

static void handle_options(char **envp, struct link_map *map)
{
    ElfW(auxv_t) *auxv = find_auxv(envp);
    char *show_auxvp = get_env_value(envp, "LD_SHOW_AUXV");
    if (show_auxvp)
        print_auxvp(auxv);

    char *trace_ld_objects = get_env_value(envp, "LD_TRACE_LOADED_OBJECTS");
    if (trace_ld_objects)
        while(map)
        {
            printf("\t%s (0x%016lx)\n", map->l_name, map->l_addr);
            map = map->l_next;
        }
}

void ldso_main(u64 *stack)
{
    int argc = *stack;
    char **argv = (void *)&stack[1];
    char **envp = argv + argc + 1;
    ElfW(auxv_t) *auxv = find_auxv(envp);
    int size = 0; 
    char *filename = (void *)get_auxv_entry(auxv, AT_EXECFN)->a_un.a_val;
    void *file = read_elf_file(filename, &size);
    struct link_map *map = build_link_map(envp, file);
    
    handle_options(envp, map);
    

    u64 entry = get_auxv_entry(auxv, AT_ENTRY)->a_un.a_val;
    jmp_to_usercode(entry, (u64)stack);
    _exit(0);
}
