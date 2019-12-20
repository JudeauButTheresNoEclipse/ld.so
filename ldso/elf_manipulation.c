#include "include/elf_manipulation.h"
#include "include/utility.h"
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
    int filedes = open(filename, O_RDONLY);
    if (filedes == -1)
    {
        printf("coult not open: %s", filename);
        _exit(1);
    }
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
    int filedes = open(name, O_RDONLY);
    if (filedes == -1)
    {
        printf("could not open: %s", name);
        _exit(1);
    }
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
    int filedes = open(name, O_RDONLY);
    if (filedes == -1)
        printf("COULD NOT OPEN: %s", name);
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
            int filedes = open (name, O_RDONLY);
            if (filedes == -1)
            {
                printf("COULD NOT OPEN: %s", name);
                _exit(0);
            }
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
    int filedes = open(name, O_RDONLY);
    if (filedes == -1)
    {
        printf("coult not open: %s", name);
        _exit(1);
    }
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
            int filedes = open (name, O_RDONLY);
            if (filedes == -1)
            {
                printf("could not open file: %s", name);
                _exit(1);
            }
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

static uint32_t gnu_hash (const char *namearg) //from binutils-gdb source
 {
    const unsigned char *name = (const unsigned char *) namearg;
    unsigned long h = 5381;
    unsigned char ch;
  
    while ((ch = *name++) != '\0')
        h = (h << 5) + h + ch;
    return h & 0xffffffff;
}

uint32_t *get_hashtab(elf_ehdr *elf, struct link_map *map)
{
    char *name = map->l_name;
    elf_dyn *dyn = (void *)get_dynamic_element(elf, name, ".dynamic");
    uint32_t *hashtab = NULL;
    while (dyn->d_tag != DT_NULL)
    {
        if (dyn->d_tag == DT_GNU_HASH)
        {
            hashtab = (void *)dyn->d_un.d_val;
            break;
        }
        dyn++;;
    }
    return hashtab;
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

elf_addr gnu_hash_lookup(struct link_map *next, char *rela_name)
{
    uint32_t h1 = gnu_hash(rela_name);
    char *name = next->l_name;
    elf_sym *sym = NULL;
    elf_ehdr *elf = get_elf_ehdr(name);
    char *strtab = (void *)get_dynamic_element(elf, name, ".dynstr");
    uint32_t *hashtab = (void *)((char *)get_hashtab(elf, next) + next->l_addr);
    if (!hashtab)
        return 0;
    elf_sym *syms = (void *)get_dynamic_element(elf, name, ".dynsym");
    size_t size = get_dynsym_size(elf, name);
    uint32_t nbuckets = hashtab[0];
    size_t *maskwords = (size_t *)(hashtab + 4);
    uint32_t *buckets = hashtab + 4 + (hashtab[2] * (sizeof(size_t) /
                sizeof(uint32_t)));
    uint32_t symndx = hashtab[1];
    uint32_t shift2 = hashtab[3];
    uint32_t *hashval;
    uint32_t *hashvals = buckets + nbuckets;
    uint32_t h2 = h1 >> shift2;
    size_t arch = sizeof(size_t) * 8; //32 or 64 depending on arch
    //printf("SEARCHING IN %s\n", name);
    
    size_t n = (h1 / arch) & (hashtab[2] - 1);
    elf_addr word = maskwords[(h1 / arch) % hashtab[2]];
    elf_addr mask = 0 | (elf_addr)1 << (h1 % arch)
        | (elf_addr)1 << ((h1 >> hashtab[3]) % arch);
    if ((word & mask) != mask)
        return 0;
    n = buckets[h1 % nbuckets];
    if (!n)
        return 0;
    sym = syms + n;
    hashval = hashvals + n - symndx;
    size_t i = 0;
    for (h1 &= ~1; n + i < size; sym++, i++) 
    {
        h2 = *hashval++;
        if ((h1 == (h2 & ~1)) && !strcmp(rela_name, strtab + sym->st_name))
            return sym->st_value + next->l_addr;
        if (h2 & 1)
            continue;
    }
    return 0;
}

uint32_t elf_hash(char* name) //https://refspecs.linuxfoundation.org/elf/gabi4+/ch5.dynamic.html#hash
{
    uint32_t h = 0, g;
    for (; *name; name++) {
        h = (h << 4) + *name;
        if ((g = h) & 0xf0000000) {
            h ^= g >> 24;
        }
        h &= ~g;
    }
    return h;
}

elf_addr hash_lookup(struct link_map *next, char *rela_name)
{
    elf_ehdr *elf = get_elf_ehdr(next->l_name);
    uint32_t *hashtab = (void *)((char *)get_hashtab(elf, next) + next->l_addr);
    elf_sym *symtab = (void *)get_dynamic_element(elf, next->l_name, ".dynsym");
    char *strtab = (void *)get_dynamic_element(elf, next->l_name, ".dynstr");
    uint32_t hash = elf_hash(rela_name);
    uint32_t nbuckets = hashtab[0];
    uint32_t *bucket = hashtab + 2;
    uint32_t *chain = bucket + nbuckets;
    for (uint32_t i = bucket[hash % nbuckets]; i; i = chain[i])
    {
        if (!strcmp(rela_name, strtab + symtab[i].st_name))
            return (elf_addr)(symtab + i);
    }
    return 0;
}

elf_addr link_map_lookup(struct link_map *map, char *rela_name)
{
    //printf("NAME: %s\n", rela_name);
    for (struct link_map *next = map; next; next = next->l_next)
    {
        elf_addr res = gnu_hash_lookup(next, rela_name);
        if (res != 0)
            return res;
    }
    printf("coult not find dependency %s\n", rela_name);
    _exit(1);
    return 0; //to remove warning
}
