#include "functions.h"
#include "stdlib.h"
#include "string.h"
#include "elf_manipulation.h"

#define MAX_DEP 100

static void is_in(char **res, char *name, int nb_dep)
{
    int ret = 0;
    for (int i = 0; i < nb_dep; i++)
        if (!strcmp(res[i], name))
            ret = 1;
    return ret;
}

static void fill_dependency(char **res, int *nb_dep)
{
    elf_ehdr *elf = get_elf_ehdr(filename);
    elf_dyn *dynamic = get_dynamic_section(elf);
    while (dynamic->d_tag != DT_NULL)
    {
        if (dynamic->d_tag == DT_NEEDED)
        {
            char *name = get_dynamic_name(dynamic->d_un.d_val);        
            if (!is_in(name, res, *nb_dep))
                res[(*nb_dep)++] = name;
        }
        dynamic++;
    }
}

char **build_dependency_table(char *executable_name)
{
    char **res = malloc(MAX_DEP * sizeof(MAX_DEP));
    int nb_dep = 0;
    res[0] = executable_name;
    int cpy = 0;
    do {
        cpy = nb_dep;
        fill_dependency(res, &nb_dep);
    } while(cpy != nb_dep);

    res[nb_dep] = NULL;
    return res;
}


static void resolve_relocations(elf_ehdr *elf, struct link_map *map)
{
    elf_rela *rela = get_relocations(elf);
    int nb_rela = get_nb_rela(elf);
    for (int i = 0; i < nb_rela; i++)
    {
        if (ELF64_R_TYPE(rela->r_info) == R_X86_64_JUMP_SLOT)
        {    
            char *rela_name = name_from_dynsim_index(elf, ELF64_R_SYM(rela->r_info));
            elf_sym *symbol = link_map_lookup(map, rela_name);
            if (symbol == NULL)
                goto out;
            elf_addr *tmp = rela->offset;
            *tmp = symbol->st_value;
        }
    }
}

void load_program(elf_phdr *program, elf_ehdr *elf, char *name, struct link_map *map)
{

    int size = elf->e_phnum;
    int prot = PROT_EXEC | PROT_READ | PROT_WRITE;
    int flags = MAP_FIXED;
    for (int i = 0; i < size; i++)
    {
        if ((phdr + i)->pt_type == PT_INTERP) //if the program is the executable
            load_interp(phdr, elf);           //we load interp
        else if ((phdr + i)->pt_type == PT_LOAD)
        {
            int filedes = open(name, O_RDWR);
            if (files == -1)
                goto out;
            elf_addr addr = mmap(program->p_vaddr, program->p_filesz, prot, 
                    flags, filedes, program->offset);
            if (addr == MAP_FAILED)
                goto out;
            close(filedes);
        }
        else
            break;
        program++;
    }
    resolve_relocations(elf, map);
}


int load_elf_binary(char *name, elf_auxv_t *auxv, struct link_map *map)
{
    elf_ehdr *elf = get_elf_ehdr(name);
    elf_phdr *phdr = get_program_header(elf);
    load_program(phdr, elf, name, map);
}
