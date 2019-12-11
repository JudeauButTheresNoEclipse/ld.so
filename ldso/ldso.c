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

static void build_link_map()
{
    char *filename = (void *)get_auxv_entry(auxv, AT_EXECFN)->a_un.a_val;
    void *file = read_elf_file(filename, &size);
    if (file == MAP_FAILED)
        return;
     get_needed_entry(dynamic, map);
     ElfW(Dyn) *dynamic = get_dynamic_section();
}


static void handle_options(int argc, char *argv[], char **envp)
{
    (void) argc;
    (void) argv;
    ElfW(auxv_t) *auxv = find_auxv(envp);
    char *show_auxvp = get_env_value(envp, "LD_SHOW_AUXV");
    if (show_auxvp)
        print_auxvp(auxv);
    char *trace_ld_objects = get_env_value(envp, "LD_TRACE_LOADED_OBJECTS");
    struct link_map *map = build_link_map();
    if (trace_ld_objects)
    {
        int size = 0;
        while(map->l_next)
        {
            printf("\t%s =>", map->l_name);
            map = map->l_next;
        }
    }
}

void ldso_main(u64 *stack)
{
    int argc = *stack;
    char **argv = (void *)&stack[1];
    char **envp = argv + argc + 1;
    handle_options(argc, argv, envp);
    //ElfW(auxv_t) *auxv = find_auxv(envp);
    //u64 entry = get_auxv_entry(auxv, AT_ENTRY)->a_un.a_val;
    _exit(0);
    //jmp_to_usercode(entry, (u64)stack);
}
