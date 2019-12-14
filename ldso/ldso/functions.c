#include "functions.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "elf_manipulation.h"
#include "unistd.h"

#include <sys/auxv.h>
#include <linux/mman.h>
#include <asm-generic/fcntl.h>
#include <errno.h>

#define MAX_DEP 100
#define PAGE_SIZE 4096

static int is_in(char **res, char *name, int nb_dep)
{
    int ret = 0;
    for (int i = 0; i < nb_dep; i++)
        if (!strcmp(res[i], name))
            ret = 1;
    return ret;
}

static void fill_dependency(char **res, int *nb_dep)
{
    for (int i = 0; i < *nb_dep; i++)
    {   
        elf_ehdr *elf = get_elf_ehdr(res[i]);
        elf_dyn *dynamic = get_dynamic_section(elf, res[i]);
        while (dynamic->d_tag != DT_NULL)
        {
            if (dynamic->d_tag == DT_NEEDED)
            {
                char *name = get_dynamic_name(dynamic->d_un.d_val, res[i]);
                if (!is_in(res, name, *nb_dep))
                    res[(*nb_dep)++] = name;
            }
            dynamic++;
        }
    }
}

char **build_dependency_table(char *executable_name)
{
    char **res = malloc(MAX_DEP * sizeof(char *));
    int nb_dep = 1;
    res[0] = executable_name;
    int cpy = 0;
    do {
        cpy = nb_dep;
        fill_dependency(res, &nb_dep);
    } while(cpy != nb_dep);

    res[nb_dep] = NULL;
    return res;
}

struct link_map *build_link_map(char **table, elf_addr interp)
{
    struct link_map *map = NULL;
    struct link_map *ret = NULL;
    int offset = 0;
    int count = 0;
    while (*table)
    {
        struct link_map *new = malloc(sizeof(struct link_map)); 
        new->l_name = *table;
        
        
        new->l_next = NULL;
        new->l_prev = map;
        if (map)
        {
            new->l_addr = offset + count * PAGE_SIZE;
            elf_ehdr *elf = get_elf_ehdr(new->l_name);
            elf_phdr *program = get_program_header(elf, new->l_name);
            load_program(program, elf, new, interp);
            free(elf);
            map->l_next = new;
        }
        else
        {
            new->l_addr = 0;
            offset = load_elf_binary(new, interp);
            ret = new;
        }
        map = new; 
        table++;
        count++;
    }
    return ret;
}

void resolve_relocations(char *name, struct link_map *map)
{
    elf_ehdr *elf = get_elf_ehdr(name);
    elf_rela *rela = get_relocations(elf, name);
    int nb_rela = get_nb_rela(elf, name);
    for (int i = 0; i < nb_rela; i++)
    {
        if (ELF64_R_TYPE(rela->r_info) == R_X86_64_JUMP_SLOT)
        {    
            char *rela_name = name_from_dynsim_index(elf, name, ELF64_R_SYM(rela->r_info));
            elf_sym *symbol = link_map_lookup(map, rela_name);
            //if (symbol == NULL)
            //    goto out;
            elf_addr *tmp = (void *)(rela->r_offset);
            *tmp = symbol->st_value;
        }
    }
    free(rela);
}

static void load_interp(elf_addr interp)
{
    char *name = "ld.so";
    elf_ehdr *elf = get_elf_ehdr(name);
    elf_phdr *program = get_program_header(elf, name);
    int size = elf->e_phnum;
    int prot = PROT_EXEC | PROT_READ | PROT_WRITE;
    int flags = MAP_PRIVATE | MAP_FIXED;
    for (int i = 0; i < size; i++)
    {
        if (program->p_type == PT_LOAD)
        {
            int filedes = open(name, O_RDONLY);
            elf_addr addr = mmap(interp + program->p_vaddr, program->p_filesz, 
                    prot, flags, filedes, program->p_offset);
            printf("%lx, %d, %d\n", program->p_vaddr, program->p_filesz, program->p_offset);
            printf("%lx\n", addr);
            close(filedes);
        }
        program++;
    }
    free(elf);
}

void load_program(elf_phdr *program, elf_ehdr *elf, struct link_map *map, elf_addr interp)
{

    int size = elf->e_phnum;
    int prot = PROT_EXEC | PROT_READ | PROT_WRITE;
    int flags = MAP_FIXED | MAP_PRIVATE;
    int load = 0;
    for (int i = 0; i < size; i++)
    {
        printf("%d : %d\n", program->p_type, PT_LOAD);
        if (program->p_type == PT_INTERP) //if the program is the executable
            load_interp(interp);                //we load interp
        else if (program->p_type == PT_LOAD)
        {
            printf("%s\n", map->l_name);
            int filedes = open(map->l_name, O_RDONLY);
            printf("%d\n", filedes);
            //if (files == -1)
            //    goto out;
            int l = lseek(filedes, program->p_offset, SEEK_SET);
            elf_addr addr = program->p_vaddr + map->l_addr;
            int r = read(filedes, (void *)addr, program->p_filesz);
            printf("l : %d, r : %d\n", l, r);
            //elf_addr addr = mmap(program->p_vaddr, program->p_filesz, prot,
            //        flags, filedes, program->p_offset);
            printf("%lx, %d, %d\n", program->p_vaddr, program->p_filesz, program->p_offset);
            printf("%lx\n", addr);
            //if (addr == MAP_FAILED)
            //    goto out;
            close(filedes);
            load = 1;
        }
        else if (load == 1)
            break;
        program++;
    }
}

static elf_addr allocate_map(elf_phdr *phdr)
{
    int prot = PROT_EXEC | PROT_READ | PROT_WRITE;
    int flags = MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS;
    while (phdr->p_type != PT_LOAD)
        phdr++;
    return mmap(phdr->p_vaddr, 7 * PAGE_SIZE, prot, flags, -1, 0);
}


elf_addr load_elf_binary(struct link_map *map, elf_addr interp)
{
    elf_ehdr *elf = get_elf_ehdr(map->l_name);
    elf_phdr *phdr = get_program_header(elf, map->l_name);
    elf_addr ret = allocate_map(phdr);
    printf("MAP: 0x%016lx\n", ret);
    load_program(phdr, elf, map, interp);
    free(elf);
    return ret;
}
