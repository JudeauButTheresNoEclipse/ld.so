#include <elf.h>
#include <link.h>
#include <stddef.h>
#include <sys/auxv.h>
#include <linux/mman.h>
#include <asm-generic/fcntl.h>

#include "string.h"
#include "types.h"
#include "stdio.h"
#include "unistd.h"
#include "display_auxv.h"

#define STR 0
#define HEX 1
#define DEC 2

char *get_env_value(char **envp, char *name)
{
    int name_size = strlen(name);
    for (size_t i = 0; envp[i]; i++)
    {
        int size = strlen(envp[i]);
        if (name_size < size && !strncmp(envp[i], name, name_size))
            return envp[i] + name_size + 1;
    }
    return NULL;
}

void print_env_values(char **envp)
{
    for (size_t i = 0; envp[i]; i++)
        printf("%s\n", envp[i]);
}

static void print_option(ElfW(auxv_t) * auxv)
{
    struct dictionary_entry
    {
        ElfW(Addr) a_type;
        char msg[30];
        int type;
    } auxv_options[] = { { AT_BASE, "AT_BASE", HEX },
                         { AT_BASE_PLATFORM, "AT_BASE_PLATFORM", STR },
                         { AT_CLKTCK, "AT_CLKTCK", DEC },
                         { AT_DCACHEBSIZE, "AT_DCACHEBSIZE", DEC },
                         { AT_EGID, "AT_EGID", DEC },
                         { AT_ENTRY, "AT_ENTRY", HEX },
                         { AT_EUID, "AT_EUID", DEC },
                         { AT_EXECFD, "AT_EXECFD", DEC },
                         { AT_EXECFN, "AT_EXECFN", STR },
                         { AT_FLAGS, "AT_FLAGS", HEX },
                         { AT_FPUCW, "AT_FPUCW", HEX },
                         { AT_GID, "AT_GID", DEC },
                         { AT_HWCAP, "AT_HWCAP", HEX },
                         { AT_HWCAP2, "AT_HWCAP2", HEX },
                         { AT_ICACHEBSIZE, "AT_ICACHEBSIZE", DEC },
                         { AT_L1D_CACHEGEOMETRY, "AT_L1D_CACHEGEOMETRY", HEX },
                         { AT_L1D_CACHESIZE, "AT_L1D_CACHESIZE", DEC },
                         { AT_L1I_CACHEGEOMETRY, "AT_L1I_CACHEGEOMETRY", HEX },
                         { AT_L1I_CACHESIZE, "AT_L1I_CACHESIZE", DEC },
                         { AT_L2_CACHEGEOMETRY, "AT_L2_CACHEGEOMETRY", HEX },
                         { AT_L2_CACHESIZE, "AT_L2_CACHESIZE", DEC },
                         { AT_L3_CACHEGEOMETRY, "AT_L3_CACHEGEOMETRY", HEX },
                         { AT_L3_CACHESIZE, "AT_L3_CACHESIZE", DEC },
                         { AT_PAGESZ, "AT_PAGESZ", DEC },
                         { AT_PHDR, "AT_PHDR", HEX },
                         { AT_PHENT, "AT_PHENT", DEC },
                         { AT_PHNUM, "AT_PHNUM", DEC },
                         { AT_PLATFORM, "AT_PLATFORM", STR },
                         { AT_RANDOM, "AT_RANDOM", HEX },
                         { AT_SECURE, "AT_SECURE", DEC },
                         { AT_SYSINFO, "AT_SYSINFO", HEX },
                         { AT_SYSINFO_EHDR, "AT_SYSINFO_EHDR", HEX },
                         { AT_UCACHEBSIZE, "AT_UCACHEBSIZE", DEC },
                         { AT_UID, "AT_UID", DEC } };
    size_t size = sizeof(auxv_options) / sizeof(auxv_options[0]);
    for (size_t i = 0; i < size; i++)
        if (auxv->a_type == auxv_options[i].a_type)
        {
            if (auxv_options[i].type == DEC)
                printf("%s : %ld\n", auxv_options[i].msg, auxv->a_un.a_val);
            if (auxv_options[i].type == HEX)
                printf("%s : 0x%lx\n", auxv_options[i].msg, auxv->a_un.a_val);
            if (auxv_options[i].type == STR)
                printf("%s : %s\n", auxv_options[i].msg, (char *)auxv->a_un.a_val);
        }
}

void print_auxvp(ElfW(auxv_t) * auxv)
{
    for (; auxv->a_type != AT_NULL; auxv++)
        print_option(auxv);
}
