#include "include/loader.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "include/elf_manipulation.h"
#include "unistd.h"
#include "libdl.h"
#include "include/utility.h"

#include <sys/auxv.h>
#include <linux/mman.h>
#include <asm-generic/fcntl.h>
#include <errno.h>
#include <link.h>

#define PAGE_SIZE 4096
#define ALIGN(x) (PAGE_SIZE * (x / PAGE_SIZE))

void _debug(void)
{
}

struct link_map *build_link_map(char **table, elf_addr base, elf_addr vdso, struct link_map *map)
{
    struct link_map *ret = NULL;
    int offset = 0;
    int count = 1;
    while (*table)
    {
        struct link_map *new = NULL;
        if (offset != 0)
        {
            new = xmalloc(sizeof(struct link_map));
            new->l_prev = map;
        }
        else
        {
            new = map;
            new->l_prev = NULL;
        }

        new->l_next = NULL;
        new->l_name = *table;
        if (!strcmp(*table, "ld.so"))
        {
            new->l_addr = base;
            map->l_next = new;
        }
        else if (!strcmp(*table, "linux-vdso.so.1"))
        {
            new->l_addr = vdso;
            map->l_next = new;
        }
        else if (offset != 0)
        {
            new->l_addr = offset + count * PAGE_SIZE;
            elf_ehdr *elf = get_elf_ehdr(new->l_name);
            elf_phdr *program = get_program_header(elf, new->l_name);
            load_program(program, elf, new);
            free(elf);
            free(program);
            map->l_next = new;
            offset = new->l_addr;
        }
        else
        {
            new->l_addr = 0;
            offset = load_elf_binary(new);
            ret = new;
        }
        map = new; 
        table++;
        count += 6;
    }
    full_map = ret;
    return ret;
}


static elf_addr allocate_map(elf_phdr *phdr, elf_off offset)
{
    int prot = PROT_EXEC | PROT_READ | PROT_WRITE;
    int flags = MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS;
    elf_addr ret = 0;
    while (phdr->p_type != PT_LOAD)
        phdr++;
    elf_phdr *prog = phdr;
    while (prog->p_type == PT_LOAD)
        prog++;
    elf_addr pos = phdr->p_vaddr + offset;
    size_t size = prog->p_vaddr + prog->p_filesz - phdr->p_vaddr + pos - ALIGN(pos);
    ret = (elf_addr)mmap((void *)(ALIGN(pos)),
            size, prot, flags, -1, 0);
    if ((void *)ret == MAP_FAILED)
    {
        printf("failed allocate map\n");
        _exit(1);
    }
    return ret;
}

static int prot (uint32_t flags)
{
    int res = 0;
    if (flags & PF_X)
        res |= PROT_EXEC;
    if (flags & PF_W)
        res |= PROT_WRITE;
    if (flags & PF_R)
        res |= PROT_READ;
    return res;
}

elf_addr load_program(elf_phdr *program, elf_ehdr *elf, struct link_map *map)
{

    int size = elf->e_phnum;
    int load = 0;
    elf_addr ret = allocate_map(program, map->l_addr);
    for (int i = 0; i < size; i++)
    {
        if (program->p_type == PT_LOAD)
        {
            int filedes = xopen(map->l_name, O_RDONLY);
            xlseek(filedes, program->p_offset, SEEK_SET);
            elf_addr addr = program->p_vaddr + map->l_addr;
            long r = xread(filedes, (void *)addr, program->p_filesz);
            close(filedes);
            load = 1;
            int m = mprotect((void *)ALIGN(addr), r + addr - ALIGN(addr), prot(program->p_flags));
            if (m < 0)
            {
                printf("incorrect permissions for segment %d in %s\n", i, map->l_name);
                _exit(0);
            }
        }
        else if (program->p_type == PT_DYNAMIC)
            map->l_ld = (elf_dyn *)(program->p_vaddr + map->l_addr);
        else if (load == 1)
            break;
        program++;
    }
    return ret;
}

elf_addr load_elf_binary(struct link_map *map)
{
    elf_ehdr *elf = get_elf_ehdr(map->l_name);
    elf_phdr *phdr = get_program_header(elf, map->l_name);
    elf_addr ret = load_program(phdr, elf, map);
    free(elf);
    return ret;
}

