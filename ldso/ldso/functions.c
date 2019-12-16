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

char *get_env(char *name);
elf_auxv_t *get_vdso(void);
char *strcat(char *dest, char *src)
{
    int i, j;
    for (i = strlen(dest), j = 0; j < strlen(src); j++, i++)
        dest[i] = src[j];
}

static int is_in(char **res, char *name, int nb_dep)
{
    int ret = 0;
    for (int i = 0; i < nb_dep; i++)
        if (!strcmp(res[i], name))
            ret = 1;
    return ret;
}

char *concatenate(char *path, char *name)
{
        char *ret = malloc(strlen(path) + strlen(name) + 1);
        memcpy(ret, path, strlen(path));
        if (ret[strlen(ret) - 1] != '/')
            strcat(ret, "/");
        strcat(ret, name);
        return ret;
}

int test(char *path, char *name)
{
    char *ret = concatenate(path, name);
    int o = open(ret, 0);
    if (o == -1)
    {
        free(ret);
        return 0;
    }
    close(o);
    puts(ret);
    free(ret);
    return 1;
}

char *get_lib_absolute_path(char *name, char *binary)
{
    if (name[0] == '/')
        return name;
    if (test(".", name))
        return name;
    elf_ehdr *elf = get_elf_ehdr(binary);
    elf_dyn *dyn = get_dynamic_section(elf, binary);
    elf_dyn *cpy = dyn;
    char *path = NULL;
    char *ret = NULL;
    char *string = (void *)get_dynamic_element(elf, binary, ".dynstr");
    while (dyn->d_tag != DT_NULL)
    {
        if (dyn->d_tag == DT_RPATH)
        {
            path = string + dyn->d_un.d_val;
            goto out;
        }
        dyn++;
    }
    path = get_env("LD_LIBRARY_PATH");
    if (path && test(path, name))
        goto out;
    dyn = cpy;
    while (dyn->d_tag != DT_NULL)
    {
        if (dyn->d_tag == DT_RUNPATH)
        {
            path = string + dyn->d_un.d_val;
            if (test(path, name))
                goto out;
        }
        dyn++;
    }
    if (test("/lib/", name))
    {
        path = "/lib/";
        goto out;

    }
    if (test("/usr/lib", name))
    {
        path = "/usr/lib";
        goto out;
    }
    else
    {
        printf("could not find library %s needed by %s in the system", name, binary);
        _exit(1);
    }
out:
    ret = concatenate(path, name);
    free(cpy);
    free(elf);
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
                name = get_lib_absolute_path(name, res[i]);
                if (!is_in(res, name, *nb_dep))
                    res[(*nb_dep)++] = name;
            }
            dynamic++;
        }
        free(elf);
        free(dynamic);
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
    res[nb_dep++] = "linux-vdso.so.1";
    res[nb_dep] = NULL;
    return res;
}

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
        if (!strcmp(*table, "linux-vdso.so.1"))
        {
            new->l_addr = vdso;
            map->l_next = new;
        }
        else if (map)
        {
            new->l_addr = offset + count * PAGE_SIZE;
            elf_ehdr *elf = get_elf_ehdr(new->l_name);
            elf_phdr *program = get_program_header(elf, new->l_name);
            load_program(program, elf, new, 0);
            free(elf);
            free(program);
            map->l_next = new;
        }
        else
        {
            new->l_addr = 0;
            offset = load_elf_binary(new, base);
            ret = new;
        }
        map = new; 
        table++;
        count += 5;
    }
    return ret;
}

void resolve_relocations(struct link_map *next, struct link_map *map)
{
    elf_ehdr *elf = get_elf_ehdr(next->l_name);
    elf_rela *rela = get_relocations(elf, next->l_name);
    if (!rela)
        return;
    if (!rela)
    {
        free(elf);
        return;
    }
    int nb_rela = get_nb_rela(elf, next->l_name);
    printf("\n\nND_RELA: %d : %s\n", nb_rela, next->l_name);
    for (int i = 0; i < nb_rela; i++)
    {
        if (ELF64_R_TYPE(rela->r_info) == R_X86_64_JUMP_SLOT)
        {    
            char *rela_name = name_from_dynsim_index(elf, next->l_name,
                    ELF64_R_SYM(rela->r_info));
            elf_addr addr = link_map_lookup(map, rela_name);
            if (addr)
            {
                printf("RELA: %lx, %s\n", rela->r_offset + next->l_addr, rela_name);
                printf("%lx\n", addr);
                elf_addr *tmp = (void *)(rela->r_offset + next->l_addr);
                *tmp = addr;
            }
        }
        /*if (ELF64_R_TYPE(rela->r_info) == R_X86_64_JUMP_SLOT)
        {
        
        }*/

        rela++;
    }
    free(elf);
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
    printf("ALLOCATION %lx, %ld\n", ret, size);
    if ((void *)ret == MAP_FAILED)
    {
        printf("MAP FAILED ALLOCATE MAP\n");
        _exit(1);
    }
    return ret;
}

static void load_interp(elf_addr interp)
{
    char *name = "ld.so";
    elf_ehdr *elf = get_elf_ehdr(name);
    elf_phdr *program = get_program_header(elf, name);
    printf("ASSOLA %p\n", interp);
    int size = elf->e_phnum;
    for (int i = 0; i < size; i++)
    {
        if (program->p_type == PT_LOAD)
        {
            elf_phdr *prog = program;
            while (prog->p_type == PT_LOAD)
                prog++;
            size_t alloc_size = (size_t)(prog->p_vaddr + prog->p_filesz - program->p_vaddr);
            printf("SIZE: %d\n", alloc_size);
            int filedes = open(name, O_RDONLY);
            /*int l = lseek(filedes, program->p_offset, SEEK_SET);
            elf_addr addr = program->p_vaddr + interp;
            long r = read(filedes, (void *)addr, program->p_filesz);
            if (l == -1 || r == -1)
            {
                printf("failed to load interp");
                _exit(1);
            }*/
            free(elf);
            free(program);
            elf_addr addr = (elf_addr)mmap((void *)interp,
                    alloc_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                    MAP_FIXED_NOREPLACE | MAP_PRIVATE, filedes, 0);
            printf("INTERP ALLOCATION %lx, %lx, %ld\n", interp, addr, alloc_size);
            close(filedes);
            if ((void *)addr == MAP_FAILED)
            {
                printf("interp allocation failed\n");
                _exit(1);
            }
            return;
        }
        program++;
    }
    free(elf);
}

elf_addr load_program(elf_phdr *program, elf_ehdr *elf, struct link_map *map, elf_addr interp)
{

    int size = elf->e_phnum;
    int load = 0;
    elf_addr ret = allocate_map(program, map->l_addr);
    //printf("MAP: 0x%016lx\n", ret);
    for (int i = 0; i < size; i++)
    {
        if (program->p_type == PT_INTERP) //if the program is the executable
            load++;
            //load_interp(interp);                //we load interp
        else if (program->p_type == PT_LOAD)
        {
            //printf("%s\n", map->l_name);
            int filedes = open(map->l_name, O_RDONLY);
            //if (files == -1)
            //    goto out;
            int l = lseek(filedes, program->p_offset, SEEK_SET);
            elf_addr addr = program->p_vaddr + map->l_addr;
            long r = read(filedes, (void *)addr, program->p_filesz);
            close(filedes);
            if (l == -1 || r == -1)
            {
                printf("failed to load interp");
                _exit(1);
            }
            //printf("l : %d, r : %ld\n", l, r);
            //printf("%lx, %d, %d\n", addr, program->p_filesz, program->p_offset);
            //printf("DECALAGE: %lx\n", map->l_addr);
            //printf("%lx\n", addr);
            load = 1;
        }
        else if (load == 1)
            break;
        program++;
    }
    return ret;
}

elf_addr load_elf_binary(struct link_map *map, elf_addr base)
{
    elf_ehdr *elf = get_elf_ehdr(map->l_name);
    elf_phdr *phdr = get_program_header(elf, map->l_name);
    elf_addr ret = load_program(phdr, elf, map, base);
    free(elf);
    return ret;
}
