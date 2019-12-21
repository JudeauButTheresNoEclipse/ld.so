#include "include/symbol_resolution.h"
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


uint32_t *get_hashtab(elf_ehdr *elf, struct link_map *map, int type)
{
    char *name = map->l_name;
    elf_dyn *dyn = (void *)get_section(elf, name, ".dynamic");
    uint32_t *hashtab = NULL;
    while (dyn->d_tag != DT_NULL)
    {
        if (dyn->d_tag == type)
        {
            hashtab = (void *)dyn->d_un.d_val;
            break;
        }
        dyn++;;
    }
    return hashtab;
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

elf_addr gnu_hash_lookup(struct link_map *next, char *rela_name)
{
    uint32_t h1 = gnu_hash(rela_name);
    char *name = next->l_name;
    elf_sym *sym = NULL;
    elf_ehdr *elf = get_elf_ehdr(name);
    char *strtab = (void *)get_section(elf, name, ".dynstr");
    uint32_t *hashtab = (void *)((char *)get_hashtab(elf, next, DT_GNU_HASH) + next->l_addr);
    if (!hashtab)
        return 0;
    elf_sym *syms = (void *)get_section(elf, name, ".dynsym");
    size_t size = get_section_size(elf, name, ".dynsym");
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
    uint32_t *hashtab = (void *)((char *)get_hashtab(elf, next, DT_HASH) + next->l_addr);
    if (!hashtab)
        return 0;
    elf_sym *symtab = (void *)get_section(elf, next->l_name, ".dynsym");
    char *strtab = (void *)get_section(elf, next->l_name, ".dynstr");
    uint32_t hash = elf_hash(rela_name);
    uint32_t nbuckets = hashtab[0];
    uint32_t *bucket = hashtab + 2;
    uint32_t *chain = bucket + nbuckets;
    size_t size = get_section_size(elf, next->l_name, ".dynsym");
    for (uint32_t i = bucket[hash % nbuckets]; i && i < size; i = chain[i])
    {
        if (!strcmp(rela_name, strtab + symtab[i].st_name))
            return symtab[i].st_value + next->l_addr;
    }
    return 0;
}

elf_addr link_map_lookup(struct link_map *map, char *rela_name)
{
    for (struct link_map *next = map; next; next = next->l_next)
    {
        elf_addr res = gnu_hash_lookup(next, rela_name);
        if (res)
            return res;
        res = hash_lookup(next, rela_name);
        if (res)
            return res;
    }
    printf("coult not find dependency %s\n", rela_name);
    _exit(1);
    return 0; //to remove warning
}
