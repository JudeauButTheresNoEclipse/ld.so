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
#include "include/display_auxv.h"
#include "include/loader.h"
#include "include/elf_manipulation.h"
#include "include/dependency.h"
#include "include/relocations.h"
#include "libdl.h"

static elf_auxv_t *vdso;
static char **envp = NULL;
struct r_debug _r_debug;

elf_auxv_t *get_vdso(void)
{
    return vdso;
}

char *get_env(char *name)
{
    return get_env_value(envp, name);
}

elf_auxv_t *get_auxv_entry(elf_auxv_t * auxv, u32 type)
{
    for (; auxv->a_type != AT_NULL; auxv++)
        if (auxv->a_type == type)
            return auxv;
    return NULL;
}

elf_auxv_t *find_auxv(char **envp)
{
    while (*envp != NULL)
        envp++;
    envp++;
    return (elf_auxv_t *)envp;
}

static inline void jmp_to_usercode(u64 entry, u64 stack)
{
    asm volatile("mov %[stack], %%rsp\n"
                 "push %[entry]\n"
                 "ret" ::[entry] "r"(entry),
                 [stack] "r"(stack));
}

static void handle_options(char **envp, struct link_map *map)
{
    elf_auxv_t *auxv = find_auxv(envp);
    char *show_auxvp = get_env_value(envp, "LD_SHOW_AUXV");
    if (show_auxvp)
        print_auxvp(auxv);

    char *trace_ld_objects = get_env_value(envp, "LD_TRACE_LOADED_OBJECTS");
    if (trace_ld_objects)
    {
        while(map)
        {
            printf("\t%s (0x%016lx)\n", map->l_name, (elf_addr)map->l_ld);
            map = map->l_next;
        }
        _exit(0);
    }
}


void ldso_main(u64 *stack, elf_dyn *dynamic)
{
    int argc = *stack;
    char **argv = (void *)&stack[1];
    envp = argv + argc + 1;
    elf_auxv_t *auxv = find_auxv(envp);
    char *filename = (void *)get_auxv_entry(auxv, AT_EXECFN)->a_un.a_val;
    elf_addr base = get_auxv_entry(auxv, AT_BASE)->a_un.a_val;
    struct link_map *map = malloc(sizeof(struct link_map));

    struct r_debug **debug_info = NULL;
    r_debug = malloc(sizeof(struct r_debug));
    while (++dynamic->d_tag != DT_NULL)
        if (dynamic->d_tag == DT_DEBUG)
            break;
    r_debug->r_state = RT_CONSISTENT; 
    r_debug->r_ldbase = (elf_addr)dynamic;
    r_debug->r_brk = (elf_addr)_debug; 
    r_debug->r_map = map;
    r_debug->r_version = 1;
    
    if (dynamic->d_tag != DT_NULL)
    {
        debug_info = (struct r_debug**)&(dynamic->d_un.d_ptr);
        *debug_info = r_debug;
    }
  
    vdso = (void *)get_auxv_entry(auxv, AT_SYSINFO_EHDR);
    char **table = build_dependency_table(filename, envp, vdso);
    map = build_link_map(table, base, (elf_addr)vdso, map);


    int lazy = 1;
    if (get_env_value(envp, "LD_BIND_NOW") != NULL)
        lazy = 0;
    for (struct link_map *next = map; next->l_next; next = next->l_next)
        resolve_relocations(next, map, lazy);
    
    

    handle_options(envp, map);
    

    u64 entry = get_auxv_entry(auxv, AT_ENTRY)->a_un.a_val;
    

    free(table);
    jmp_to_usercode(entry, (u64)stack);
    _exit(0);
}
