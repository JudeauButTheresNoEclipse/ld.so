#include <stdio.h>
#include <link.h>
#include <elf.h>
#include <string.h>

#include "include/print_headers.h"
#include "include/header_info.h"

#define NAME 0
#define VAL 1

static void print_name(void *dyn)
{
    ElfW(Dyn) *dynamic = dyn;
    if (dynamic->d_tag == DT_NEEDED)
    {
        printf("%s\n", (char *)get_str_tab() + dynamic->d_un.d_val);
    }
    else
        printf("\n");
}
static void print_tag(void *dyn)
{
    ElfW(Dyn) *dynamic = dyn;
    ElfW(Word) val = dynamic->d_tag;
    struct dictionary_entry
    {
        ElfW(Word) value;
        char *msg;
        int name_value;
    } tag_list[] = { { DT_NULL, "NULL", VAL },
                     { DT_NEEDED, "NEEDED", NAME },
                     { DT_PLTRELSZ, "PLTRELSZ", NAME },
                     { DT_PLTGOT, "PLTGOT", VAL },
                     { DT_HASH, "HASH", VAL },
                     { DT_STRTAB, "STRTAB", VAL },
                     { DT_SYMTAB, "SYMTAB", VAL },
                     { DT_RELA, "RELA", VAL },
                     { DT_RELASZ, "RELASZ", VAL },
                     { DT_RELAENT, "RELAENT", VAL },
                     { DT_STRSZ, "STRSZ", VAL },
                     { DT_SYMENT, "SYMENT", VAL },
                     { DT_INIT, "INIT", VAL },
                     { DT_FINI, "FINI", VAL },
                     { DT_SONAME, "SONAME", VAL },
                     { DT_RPATH, "RPATH", VAL },
                     { DT_SYMBOLIC, "SYMBOLIC", VAL },
                     { DT_REL, "REL", VAL },
                     { DT_RELSZ, "RELSZ", VAL },
                     { DT_RELENT, "RELENT", VAL },
                     { DT_PLTREL, "PLTREL", VAL },
                     { DT_DEBUG, "DEBUG", VAL },
                     { DT_TEXTREL, "TEXTREL", VAL },
                     { DT_JMPREL, "JMPREL", VAL },
                     { DT_BIND_NOW, "BIND_NOW", VAL },
                     { DT_RUNPATH, "RUNPATH", VAL },
                     { DT_FLAGS_1, "FLAGS_1", NAME },
                     { DT_INIT_ARRAY, "INIT_ARRAY", VAL },
                     { DT_FINI_ARRAY, "FINI_ARRAY", VAL },
                     { DT_INIT_ARRAYSZ, "INIT_ARRAYSZ", VAL },
                     { DT_FINI_ARRAYSZ, "FINI_ARRAYSZ", VAL },
                     { DT_VERNEED, "VERNEED", VAL },
                     { DT_VERNEEDNUM, "VERNEEDNUM", VAL },
                     { DT_VERSYM, "VERSYM", VAL },
                     { DT_RELACOUNT, "RELACOUNT", VAL },
                     { DT_GNU_HASH, "GNU_HASH", VAL } };
    size_t size = sizeof(tag_list) / sizeof(tag_list[0]);
    for (size_t i = 0; i < size; i++)
        if (val == tag_list[i].value)
        {
            if (tag_list[i].name_value == NAME)
            {
                printf("(%s) ", tag_list[i].msg);
                print_name(dynamic);
            }
            else
            {
                if (get_elf_class() == ELFCLASS64)
                    printf("(%s) 0x%lx\n", tag_list[i].msg, dynamic->d_un.d_val);
                else
                    printf("(%s) 0x%x\n", tag_list[i].msg,
                           (uint32_t)dynamic->d_un.d_val);
            }
        }
}

void print_dynamic_section()
{
    ElfW(Dyn) *dynamic = get_dynamic_section();
    printf("\n\nDynamic section at offset 0x%lx:\n", (void *)dynamic - get_elf_header());
    printf(" Tag        Type                         Name/Value\n");
    while (dynamic->d_tag != DT_NULL)
    {
        printf("0x%016lx ", dynamic->d_tag);
        print_tag(dynamic);
        dynamic++;
    }
}
