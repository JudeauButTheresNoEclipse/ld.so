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
    elf_ehdr *elf = xmalloc(sizeof(elf_ehdr));
    xread(filedes, (char *)elf, sizeof(elf_ehdr));
    close(filedes);
    return elf;
}

elf_phdr *get_program_header(elf_ehdr *elf, char *name)
{
    int size = elf->e_phnum * sizeof(elf_phdr);
    int filedes = xopen(name, O_RDONLY);
    elf_phdr *phdr = xmalloc(size);
    xlseek(filedes, elf->e_phoff, SEEK_SET);
    xread(filedes, (void *)phdr, size);
    close(filedes);
    return phdr;
}


elf_shdr *get_section_header(elf_ehdr *elf, char *name)
{
    int size = elf->e_shnum * sizeof(elf_shdr);
    int filedes = xopen(name, O_RDONLY);
    elf_shdr *shdr = xmalloc(size);
    xlseek(filedes, elf->e_shoff, SEEK_SET);
    xread(filedes, (void *)shdr, size);
    close(filedes);
    return shdr;
}

char *get_string_table(elf_ehdr *elf, char *name)
{
    elf_shdr *shdr = get_section_header(elf, name);
    elf_shdr *tmp = shdr + elf->e_shstrndx;
    int filedes = xopen(name, O_RDONLY);
    char *string_table = xmalloc(tmp->sh_size);
    xlseek(filedes, tmp->sh_offset, SEEK_SET);
    xread(filedes, string_table, tmp->sh_size);
    close(filedes);
    return string_table;
}

elf_sym *get_section(elf_ehdr *elf, char *name, char *elt)
{
    elf_shdr *shdr = get_section_header(elf, name);
    elf_shdr *cpy = shdr;
    char *string_table = get_string_table(elf, name);
    elf_sym *section = NULL;
    for (int i = 0; i < elf->e_shnum; i++)
    {
        if (!strcmp(string_table + shdr->sh_name, elt))
        {
            int filedes = xopen (name, O_RDONLY);
            section = xmalloc(shdr->sh_size);
            xlseek(filedes, shdr->sh_offset, SEEK_SET);
            xread(filedes, (void *)section, shdr->sh_size);
            close(filedes);
            break;
        }
        shdr++;
    }
    free(cpy);
    free(string_table);
    return section;
}

char *name_from_dynsim_index(elf_ehdr *elf,  char *name, int index)
{

    elf_sym *symbolic = get_section(elf, name, ".dynsym");
    elf_sym *cpy = symbolic;
    for (int i = 0; i < index; i++)
        symbolic++;
    char *dynstr = (void *)get_section(elf, name, ".dynstr");
    char *res = strdup(dynstr + symbolic->st_name);
    free(cpy);
    free(dynstr);
    return res;
}

int get_section_size(elf_ehdr *elf, char *name, char *section)
{
    elf_shdr *shdr = get_section_header(elf, name);
    elf_shdr *tmp = shdr;
    char *table = get_string_table(elf, name);
    int res = 0;
    for (int i = 0; i < elf->e_shnum; i++)
    {
        if (!strcmp(table + shdr->sh_name, section))
        {
            res = shdr->sh_size / sizeof(elf_sym);
            break;
        }
        shdr++;
    }
    free(tmp);
    free(table);
    return res;
}
