#include "include/functions.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "include/elf_manipulation.h"
#include "unistd.h"
#include "include/utility.h"

#include <sys/auxv.h>
#include <linux/mman.h>
#include <asm-generic/fcntl.h>
#include <errno.h>

#define PAGE_SIZE 4096
#define ALIGN(x) (PAGE_SIZE * (x / PAGE_SIZE))

char *get_env(char *name);
elf_auxv_t *get_vdso(void);

struct link_map *build_link_map(char **table, elf_addr base, elf_addr vdso)
{
    struct link_map *map = NULL;
    struct link_map *ret = NULL;
    int offset = 0;
    int count = 1;
    while (*table)
    {
        struct link_map *new = malloc(sizeof(struct link_map)); 
        new->l_name = *table;
        
        
        new->l_next = NULL;
        new->l_prev = map;
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
        else if (map)
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
    size_t size = (size_t)(prog->p_vaddr + prog->p_filesz - phdr->p_vaddr);
    ret = (elf_addr)mmap((void *)(phdr->p_vaddr + offset),
            size, prot, flags, -1, 0);
    printf("ALLOCATION %lx, %ld\n", phdr->p_vaddr + offset, size);
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
    //printf("MAP: 0x%016lx\n", ret);
    for (int i = 0; i < size; i++)
    {
        if (program->p_type == PT_LOAD)
        {
            int filedes = open(map->l_name, O_RDONLY);
            if (filedes == -1)
            {
                printf("could not read %s\n", map->l_name);
                _exit(1);
            }
            int l = lseek(filedes, program->p_offset, SEEK_SET);
            elf_addr addr = program->p_vaddr + map->l_addr;
            long r = read(filedes, (void *)addr, program->p_filesz);
            close(filedes);
            if (l < 0 || r < 0)
            {
                printf("failed to load program section");
                _exit(1);
            }
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

static int relocation_lookup(elf_rela *rela, int nb_rela,
        struct link_map *next, struct link_map *map)
{   
    elf_ehdr *elf = get_elf_ehdr(next->l_name);
    elf_addr addr = 0;
    for (int i = 0; i < nb_rela; i++)
    {
        if (ELF64_R_TYPE(rela->r_info) == R_X86_64_JUMP_SLOT  || 
                ELF64_R_TYPE(rela->r_info) == R_X86_64_GLOB_DAT)
        {    
            char *rela_name = name_from_dynsim_index(elf, next->l_name,
                    ELF64_R_SYM(rela->r_info));
            elf_addr *tmp = (void *)(rela->r_offset + next->l_addr);
            // printf("RELA: %lx, %s\n", rela->r_offset, rela_name);
            //printf("TMP: %lx, *TMP %lx\n", tmp, *tmp);
            addr = link_map_lookup(map, rela_name);
            if (addr)
                *tmp = addr;

        }
        if (ELF64_R_TYPE(rela->r_info) == R_X86_64_RELATIVE)
        {
            printf("%lx\n", rela->r_offset);
            elf_addr *tmp = (void *)(rela->r_offset + next->l_addr);
            printf("TMP %lx", tmp);
            *tmp = rela->r_addend + next->l_addr;
        }
        rela++;
    }
    free(elf);
    return 0;
}

static void relocation_lazy(struct link_map *next, elf_rela *rela, int nb_rela)
{
    extern void __reloc(void);
    elf_ehdr *elf = get_elf_ehdr(next->l_name);
    elf_shdr *shdr = get_section_header(elf, next->l_name);
    char *strtab = (void *)get_dynamic_element(elf, next->l_name, ".shstrtab");
    while (shdr && strcmp(shdr->sh_name + strtab, ".got.plt"))
        shdr++;
    elf_addr *got_plt = shdr->sh_addr + next->l_addr;
    got_plt[1] = (elf_addr)next;
    got_plt[2] = (elf_addr)&__reloc;
    for (int i = 0; i < nb_rela; i++, rela++)
    {
        elf_addr *tmp = (void *)(rela->r_offset + next->l_addr);
        *tmp += next->l_addr;
    }
    free(elf);
    free(shdr);
    free(strtab);
}


void resolve_relocations(struct link_map *next, struct link_map *map, int lazy)
{
    elf_ehdr *elf = get_elf_ehdr(next->l_name);
    elf_rela *rela = (elf_rela *)get_dynamic_element(elf, next->l_name, ".rela.plt");
    if (rela)
    {
        int nb_rela = get_nb_rela(elf, next->l_name);
        if (!lazy)
            relocation_lookup(rela, nb_rela, next, map);
        else
            relocation_lazy(next, rela, nb_rela);
    } 
    rela = (elf_rela *)get_dynamic_element(elf, next->l_name, ".rela.dyn");
    if (rela)
    {
        int nb_rela = get_nb_reladyn(elf, next->l_name);
        relocation_lookup(rela, nb_rela, next, map);
    }
    free(elf);
}