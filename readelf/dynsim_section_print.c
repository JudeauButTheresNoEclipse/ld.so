#include <stdio.h>
#include <link.h>
#include <elf.h>

#include "include/print_headers.h"
#include "include/header_info.h"

void print_type(unsigned char st_info)
{
    struct dictionary_entry
    {
        int type;
        char *msg;
    } type_list[] = { { STT_NOTYPE, "NOTYPE" }, { STT_OBJECT, "OBJECT" },
                      { STT_FUNC, "FUNC" },     { STT_SECTION, "SECTION" },
                      { STT_FILE, "FILE" },     { STT_LOPROC, "LOPROC" },
                      { STT_HIPROC, "HIPROC" } };
    int bind = 0;
    int type = 0;
    if (get_elf_class() == ELFCLASS32)
    {
        bind = ELF32_ST_BIND(st_info);
        type = ELF32_ST_TYPE(st_info);
    }
    else
    {
        bind = ELF64_ST_BIND(st_info);
        type = ELF64_ST_TYPE(st_info);
    }
    size_t size = sizeof(type_list) / sizeof(type_list[0]);
    for (size_t i = 0; i < size; i++)
    {
        if (type == type_list[i].type)
            printf("%s\t", type_list[i].msg);
    }
    if (bind == STB_GLOBAL)
        printf("GLOBAL\t");
    else if (bind == STB_LOCAL)
        printf("LOCAL\t");
    else if (bind == STB_WEAK)
        printf("WEEK\t");
}

static void print_vis(unsigned char st_other)
{
    int vis = 0;
    if (get_elf_class() == ELFCLASS32)
        vis = ELF32_ST_VISIBILITY(st_other);
    else
        vis = ELF64_ST_VISIBILITY(st_other);
    if (vis == STV_DEFAULT)
        printf("DEFAULT\t");
    if (vis == STV_INTERNAL)
        printf("INTERNAL\t");
    if (vis == STV_PROTECTED)
        printf("PROTECTED\t");
}

void print_dynsim_section()
{
    ElfW(Sym) *dynsim = get_dynsim_section();
    char *strtab = get_str_tab();
    printf("\n\nSymbol table '.dynsym':\n");
    printf("   Num:    Value          Size Type    Bind   Vis      Ndx Name\n");
    int size = get_dynsim_size();
    for (unsigned int i = 0; i < size / sizeof(*dynsim); i++)
    {
        printf("\t[%2d] %016lx %ld ", i, dynsim->st_value, dynsim->st_size);
        print_type(dynsim->st_info);
        print_vis(dynsim->st_other);
        if (!dynsim->st_shndx)
            printf("UND ");
        else
            printf("%d ", dynsim->st_shndx);
        printf("%s\n", strtab + dynsim->st_name);
        dynsim++;
    }
}
