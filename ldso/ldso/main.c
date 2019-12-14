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
#include "display_auxv.h"
#include "functions.h"
#include "elf_manipulation.h"

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
    elf_auxv_t *auxv = find_auxv(envp);
    
    char *filename = (void *)get_auxv_entry(auxv, AT_EXECFN)->a_un.a_val;
    char **table = build_dependency_table(filename);
    struct link_map *map = build_link_map(table);
    for (struct link_map *next = map; next; next = next->l_next)
        resolve_relocations(next, map);
    handle_options(envp, map);
    u64 entry = get_auxv_entry(auxv, AT_ENTRY)->a_un.a_val;
    printf("ENTRY: %lx\n", entry);
    jmp_to_usercode(entry, (u64)stack);
    _exit(0);
}
