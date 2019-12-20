#include "include/functions.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "include/elf_manipulation.h"
#include "unistd.h"
#include "include/utility.h"
#include "include/relocations.h"

#include <sys/auxv.h>
#include <linux/mman.h>
#include <asm-generic/fcntl.h>


static int relocation_lookup(elf_rela *rela, int nb_rela,
        struct link_map *next, struct link_map *map)
{   
    elf_ehdr *elf = get_elf_ehdr(next->l_name);
    elf_addr addr = 0;
    for (int i = 0; i < nb_rela; i++)
    {
        if (ELF64_R_TYPE(rela->r_info) == R_X86_64_JUMP_SLOT  || 
                ELF64_R_TYPE(rela->r_info) == R_X86_64_GLOB_DAT ||
                ELF64_R_TYPE(rela->r_info) == R_X86_64_RELATIVE)
        {    
            char *rela_name = name_from_dynsim_index(elf, next->l_name,
                    ELF64_R_SYM(rela->r_info));
            elf_addr *tmp = (void *)(rela->r_offset + next->l_addr);
            addr = link_map_lookup(map, rela_name);
            if (addr)
                *tmp = addr;
        }
        if (ELF64_R_TYPE(rela->r_info) == R_X86_64_RELATIVE)
        {
            elf_addr *tmp = (void *)(rela->r_offset + next->l_addr);
            *tmp = rela->r_addend + next->l_addr;
        }
        if (ELF64_R_TYPE(rela->r_info) == R_X86_64_COPY)
            continue;
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
    elf_addr *got_plt = (elf_addr *)(shdr->sh_addr + next->l_addr);
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

elf_addr runtime_relocations(struct link_map *next, int index)
{
    struct link_map *map = next;
    while (map->l_prev)
        map = map->l_prev;
    elf_ehdr *elf = get_elf_ehdr(next->l_name);
    elf_rela *rela = (elf_rela *)get_dynamic_element(elf, next->l_name, ".rela.plt");
    rela += index;
    char *rela_name = name_from_dynsim_index(elf, next->l_name,
            ELF64_R_SYM(rela->r_info));
    elf_addr *tmp = (void *)(rela->r_offset + next->l_addr);
    elf_addr addr = link_map_lookup(map, rela_name);
    if (addr)
        *tmp = addr;
    return addr;
}
