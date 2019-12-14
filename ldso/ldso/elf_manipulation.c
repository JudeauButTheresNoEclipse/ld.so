#include "elf_manipulation.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "stdio.h"

#include <sys/auxv.h>
#include <linux/mman.h>
#include <asm-generic/fcntl.h>
#include <linux/mman.h>
#include <asm-generic/fcntl.h>

static char *strdup(char *s)
{
    int len = strlen(s);
    char *res = malloc(len);
    res = memcpy(res, s, len);
    return res;
}


elf_ehdr *get_elf_ehdr(char *filename)
{
    int filedes = open(filename, O_RDONLY);
    if (filedes == -1)
        printf("COULD NOT OPEN: %s", filename);
    elf_ehdr *elf = malloc(sizeof(elf_ehdr));
    int r = read(filedes, (char *)elf, sizeof(elf_ehdr));
    if (r == -1)
    printf("read failed\n");
    close(filedes);
    return elf;
}

elf_phdr *get_program_header(elf_ehdr *elf, char *name)
{
    int size = elf->e_phnum * sizeof(elf_phdr);
    int filedes = open(name, O_RDONLY);
    if (filedes == -1)
        printf("COULD NOT OPEN: %s", name);
    elf_phdr *phdr = mmap(0, size + elf->e_phoff, PROT_READ | PROT_WRITE, MAP_PRIVATE, 
            filedes, 0);
    if (phdr == MAP_FAILED)
        printf("MMAP FAILED %s", name);
    close(filedes);
    return (void *)((char *)phdr + elf->e_phoff);
}


elf_shdr *get_section_header(elf_ehdr *elf, char *name)
{
    int size = elf->e_shnum * sizeof(elf_shdr);
    int filedes = open(name, O_RDONLY);
    if (filedes == -1)
        printf("COULD NOT OPEN: %s", name);
    elf_shdr *shdr = malloc(size);
    int l = lseek(filedes, elf->e_shoff, SEEK_SET);
    int r = read(filedes, (void *)shdr, size);
    if (r == -1 || l == -1)
        printf("COULD NOT READ SECTION HEADER %s\n", name);
    close(filedes);
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
                printf("COULD NOT OPEN: %s", name);
            dynamic = mmap(0, shdr->sh_size + shdr->sh_offset, 
                    PROT_READ | PROT_WRITE, MAP_PRIVATE, filedes, 0);
            if (dynamic == MAP_FAILED)
                printf("MMAP FAILED %s", name);
            close(filedes);
            break;
        }
        shdr++;
   }
   return (void *)((char *)dynamic + shdr->sh_offset);
}

char *get_string_table(elf_ehdr *elf, char *name)
{
    elf_shdr *shdr = get_section_header(elf, name);
    elf_shdr *tmp = shdr + elf->e_shstrndx;
    int filedes = open(name, O_RDONLY);
    if (filedes == -1)
        printf("COULD NOT OPEN: %s", name);
    char *string_table = mmap(0, tmp->sh_size + tmp->sh_offset, PROT_READ | PROT_WRITE, 
    MAP_PRIVATE, filedes, 0);
    if (string_table == MAP_FAILED)
        printf("MMAP FAILED %s", name);
    close(filedes);
    return string_table + tmp->sh_offset;
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
                printf("COULD NOT OPEN: %s", name);
            section = mmap(0, shdr->sh_size + shdr->sh_offset, PROT_READ | PROT_WRITE, 
                MAP_PRIVATE, filedes, 0);
            if (section == MAP_FAILED)
                printf("MMAP FAILED %s", name);
            close(filedes);
            break;
        }
        shdr++;
    }
    if (!section || section == MAP_FAILED)
        return NULL;
    return (void *) ((char *)section + shdr->sh_offset);
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

elf_rela *get_relocations(elf_ehdr *elf, char *name)
{
    elf_sym *relaptl = get_dynamic_element(elf, name, ".rela.plt");
    return (void *)relaptl;
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

static uint32_t gnu_hash (const char *namearg) //from binutils-gdb
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

/*elf_addr link_map_lookup(struct link_map *map, char *rela_name)
{
    printf("\n\n\nRELA_NAME: %s\n", rela_name);
    uint32_t hash = gnu_hash(rela_name);
    elf_sym *res = NULL;
    for (struct link_map *next = map; next && !res; next = next->l_next)
    {
        char *name = next->l_name;
        elf_ehdr *elf = get_elf_ehdr(name);
        char *strtab = (void *)get_dynamic_element(elf, name, ".strtab");
        uint32_t *hashtable = (void *)((char *)get_hashtab(elf, next) + next->l_addr);
        char *dynstr = (void *)get_dynstr(name);
        

        uint32_t nbuckets = hashtable[0];
        uint32_t symoffset = ((uint32_t *)hashtable)[1];
        uint32_t bloom_size = ((uint32_t *)hashtable)[2];
        uint32_t bloom_shift = ((uint32_t *)hashtable)[3];
        elf_addr *bloom = (void *)&(hashtable[4]);
        uint32_t *buckets = (void *)&bloom[bloom_size];
        uint32_t *chain = &buckets[nbuckets];
        elf_addr word = bloom[(hash / (sizeof(elf_addr) * 8)) % bloom_size];
        elf_addr mask = 0 | (elf_addr)1 << (hash % (sizeof(elf_addr) * 8))
            | (elf_addr)1 << ((hash >> bloom_shift) % (sizeof(elf_addr) * 8));

        if ((word & mask) != mask)
            continue;
        uint32_t symix = buckets[hash % nbuckets];
        if (symix < symoffset)
            continue;

        while (1)
        {
            char * symname = dynstr + symix;
            printf("SYMNAME: %s\n", symname);
            uint32_t hashing = chain[symix - symoffset];
            if (!strcmp(rela_name, symname))
            {
                return (()dynstr + symix)->st_value + next->l_addr;
            }
            if (hashing & 1)
                break;

            symix++;
        }
    }
    printf("FAILED !!!\n");
    return 0;
}*/


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

elf_addr link_map_lookup(struct link_map *map, char *rela_name)
{
    uint32_t h1 = gnu_hash(rela_name);
    //printf("NAME: %s\n", rela_name);
    elf_sym *sym = NULL;
    for (struct link_map *next = map; next; next = next->l_next)
    {
        char *name = next->l_name;
        //printf("SEARCHING IN %s\n", name);
        elf_ehdr *elf = get_elf_ehdr(name);
        char *strtab = (void *)get_dynamic_element(elf, name, ".dynstr");
        uint32_t *hashtab = (void *)((char *)get_hashtab(elf, next) + next->l_addr);
        if (!hashtab)
            continue;
        elf_sym *syms = (void *)get_dynamic_element(elf, name, ".dynsym");
        int size = get_dynsym_size(elf, name);
        uint32_t nbuckets = hashtab[0];
        size_t *maskwords = (size_t *)(hashtab + 4);
        uint32_t *buckets = hashtab + 4 + (hashtab[2] * (sizeof(size_t) /
                   sizeof(uint32_t)));
        uint32_t symndx = hashtab[1];
        uint32_t shift2 = hashtab[3];
        uint32_t h2 = h1 >> shift2;
        uint32_t *hashvals = buckets + nbuckets;
        uint32_t *hashval;
        size_t c = sizeof(size_t) * 8;
        size_t n = (h1 / c) & (hashtab[2] - 1);
        size_t bitmask = (1 << (h1 % c)) | (1 << (h2 % c));
        //if ((maskwords[n] & bitmask) != bitmask)
        //    continue;
        n = buckets[h1 % nbuckets];
        if (!n)
            continue;
        sym = syms + n;
        hashval = hashvals + n - symndx;
        int i = 0;
        for (h1 &= ~1; n + i < size; sym++, i++) {
            //if (!strcmp(rela_name, "strlen"))

            h2 = *hashval++;
            //printf("TRY %s\n", strtab +  sym->st_name);
            if ((h1 == (h2 & ~1)) && !strcmp(rela_name, strtab + sym->st_name))
            {
                printf("FOUND !!! %lx\n", sym->st_value);
                return sym->st_value + next->l_addr;
            }
            if (h2 & 1)
                continue;
            }
    }
    printf("FAILED !!!\n");
    return 0;
}
