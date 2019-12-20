#include "include/elf_manipulation.h"
#include "include/utility.h"
#include "include/symbol_resolution.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "stdio.h"

#include <sys/auxv.h>
#include <linux/mman.h>
#include <asm-generic/fcntl.h>
#include <linux/mman.h>
#include <asm-generic/fcntl.h>

elf_auxv_t *get_vdso(void);

elf_ehdr *get_elf_ehdr(char *filename)
{
    if (!strcmp(filename, "linux-vdso.so.1"))
        return (elf_ehdr *)get_vdso()->a_un.a_val;
    int filedes = xopen(filename, O_RDONLY);
    elf_ehdr *elf = malloc(sizeof(elf_ehdr));
    int r = read(filedes, (char *)elf, sizeof(elf_ehdr));
    close(filedes);
    if (r == -1)
    {
        printf("read failed %s\n", filename);
        _exit(1);
    }
    return elf;
}

elf_phdr *get_program_header(elf_ehdr *elf, char *name)
{
    int size = elf->e_phnum * sizeof(elf_phdr);
    int filedes = xopen(name, O_RDONLY);
    elf_phdr *phdr = malloc(size);
    if (!phdr)
    {
        printf("malloc failed");
        _exit(1);
    }
    int l = lseek(filedes, elf->e_phoff, SEEK_SET);
    int r = read(filedes, (void *)phdr, size);
    close(filedes);
    if (l == -1 || r == -1)
    {
        printf("read failed %s\n", name);
        _exit(1);
    }
    return phdr;
}


elf_shdr *get_section_header(elf_ehdr *elf, char *name)
{
    int size = elf->e_shnum * sizeof(elf_shdr);
    int filedes = xopen(name, O_RDONLY);
    elf_shdr *shdr = malloc(size);
    if (!shdr)
    {
        printf("malloc failed");
        _exit(1);
    }
    int l = lseek(filedes, elf->e_shoff, SEEK_SET);
    int r = read(filedes, (void *)shdr, size);
    close(filedes);
    if (r == -1 || l == -1)
    {
        printf("could not read section header %s\n", name);
        _exit(1);
    }
    return shdr;
}

elf_dyn *get_dynamic_section(elf_ehdr *elf, char *name)
{
   elf_shdr *shdr = get_section_header(elf, name);
   elf_dyn *dynamic = NULL;
   for (int i = 0; i < elf->e_shnum; i++)
   {
        if (shdr->sh_type == SHT_DYNAMIC)
        {
            int filedes = xopen (name, O_RDONLY);
            dynamic = malloc(shdr->sh_size);
            if (!dynamic)
            {
                printf("malloc failed\n");
                _exit(1);
            }
            int l = lseek(filedes, shdr->sh_offset, SEEK_SET);
            int r = read(filedes, (void *)dynamic, shdr->sh_size);
            close(filedes);
            if (r == -1 || l == -1)
            {
                printf("could not read section header %s\n", name);
                _exit(1);
            }
            break;
        }
        shdr++;
   }
   if (!dynamic)
   {
        printf("could not find dynamic section in %s\n", name);
        _exit(1);
   }
   return dynamic;
}

char *get_string_table(elf_ehdr *elf, char *name)
{
    elf_shdr *shdr = get_section_header(elf, name);
    elf_shdr *tmp = shdr + elf->e_shstrndx;
    int filedes = xopen(name, O_RDONLY);
    char *string_table = malloc(tmp->sh_size);
    int l = lseek(filedes, tmp->sh_offset, SEEK_SET);
    int r = read(filedes, string_table, tmp->sh_size);
    close(filedes);
    if (r == -1 || l == -1)
    {
        printf("could not read %s\n", name);
        _exit(1);
    }
    return string_table;
}

elf_sym *get_dynstr(char *name)
{
    elf_ehdr *elf = get_elf_ehdr(name);
    elf_sym *dynstr = (void *)get_dynamic_element(elf, name, ".dynstr");
    return dynstr;
}

char *get_dynamic_name(elf_xword val, char *name)
{
    char *dynstr = (void *)get_dynstr(name);
    char *res = strdup(dynstr + val);
    return res;
}

elf_sym *get_dynamic_element(elf_ehdr *elf, char *name, char *elt)
{
    elf_shdr *shdr = get_section_header(elf, name);
    char *string_table = get_string_table(elf, name);
    elf_sym *section = NULL;
    for (int i = 0; i < elf->e_shnum; i++)
    {
        if (!strcmp(string_table + shdr->sh_name, elt))
        {
            int filedes = xopen (name, O_RDONLY);
            section = malloc(shdr->sh_size);
            int l = lseek(filedes, shdr->sh_offset, SEEK_SET);
            int r = read(filedes, (void *)section, shdr->sh_size);
            close(filedes);
            if (l == -1 || r == -1)
            {
                printf("could not read %s\n", name);
                _exit(1);
            }
            break;
        }
        shdr++;
    }

    if (!section)
    {
        //printf("could not find section %s in file %s\n", elt, name);
        return NULL;
    }

    return section;
}

char *name_from_dynsim_index(elf_ehdr *elf,  char *name, int index)
{
    elf_sym *symbolic = get_dynamic_element(elf, name, ".dynsym");
    for (int i = 0; i < index; i++)
        symbolic++;
    char *dynstr = (void *)get_dynstr(name);
    char *res = dynstr + symbolic->st_name;
    return res;
}

int get_nb_rela(elf_ehdr *elf, char *name)
{
    elf_shdr *shdr = get_section_header(elf, name);
    char *table = get_string_table(elf, name);
    int res = 0;
    for (int i = 0; i < elf->e_shnum; i++)
    {
        if (!strcmp(table + shdr->sh_name, ".rela.plt"))
        {
            res = shdr->sh_size / sizeof(elf_sym);
            break;
        }
        shdr++;
    }
    return res;
}

int get_nb_reladyn(elf_ehdr *elf, char *name)
{
    elf_shdr *shdr = get_section_header(elf, name);
    char *table = get_string_table(elf, name);
    int res = 0;
    for (int i = 0; i < elf->e_shnum; i++)
    {
        if (!strcmp(table + shdr->sh_name, ".rela.dyn"))
        {
            res = shdr->sh_size / sizeof(elf_sym);
            break;
        }
        shdr++;
    }
    return res;
}

int get_dynsym_size(elf_ehdr *elf, char *name)
{
    elf_shdr *shdr = get_section_header(elf, name);
    char *table = get_string_table(elf, name);
    int res = 0;
    for (int i = 0; i < elf->e_shnum; i++)
    {
        if (!strcmp(table + shdr->sh_name, ".dynsym"))
        {
            res = shdr->sh_size / sizeof(elf_sym);
            break;
        }
        shdr++;
    }
    return res;
}

