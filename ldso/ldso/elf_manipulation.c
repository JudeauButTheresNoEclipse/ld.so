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
    //TODO Check return values
    elf_ehdr *elf = malloc(sizeof(elf_ehdr));
    int r = read(filedes, (char *)elf, sizeof(elf_ehdr));
    if (r != -1)
        close(filedes);
    return elf;
}

elf_phdr *get_program_header(elf_ehdr *elf, char *name)
{
    int size = elf->e_phnum * sizeof(elf_phdr);
    int filedes = open(name, O_RDONLY);
    elf_phdr *phdr = mmap(0, size + elf->e_phoff, PROT_READ | PROT_WRITE, MAP_PRIVATE, 
            filedes, 0);
    return (void *)((char *)phdr + elf->e_phoff);
}


elf_shdr *get_section_header(elf_ehdr *elf, char *name)
{
    int size = elf->e_shnum * sizeof(elf_shdr);
    int filedes = open(name, O_RDONLY);
    elf_shdr *shdr = mmap(0, size + elf->e_shoff, PROT_READ | PROT_WRITE, 
            MAP_PRIVATE, filedes, 0);
    return (void *)((char *)shdr + elf->e_shoff);
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
            dynamic = mmap(0, shdr->sh_size + shdr->sh_offset, 
                    PROT_READ | PROT_WRITE, MAP_PRIVATE, filedes, 0);
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
    char *string_table = mmap(0, tmp->sh_size + tmp->sh_offset, PROT_READ | PROT_WRITE, 
    MAP_PRIVATE, filedes, 0);
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
            section = mmap(0, shdr->sh_size + shdr->sh_offset, PROT_READ | PROT_WRITE, 
            MAP_PRIVATE, filedes, 0);
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

elf_sym *link_map_lookup(struct link_map *map, char *rela_name)
{
    printf("\n\n\nRELA_NAME: %s\n", rela_name);
    uint32_t hash = gnu_hash(rela_name);
    elf_sym *res = NULL;
    for (struct link_map *next = map; next && !res; next = next->l_next)
    {
        char *name = map->l_name;
        elf_ehdr *elf = get_elf_ehdr(name);
        char *strtab = (void *)get_dynamic_element(elf, name, ".strtab");
        elf_sym *dynstr = get_dynstr(name);
        const void *hashtable = (void *)get_dynamic_element(elf, name, ".gnu.hash");
        printf("HASHTABLE: %lx\n", *(elf_addr *)hashtable); 
        

        uint32_t nbuckets = ((uint32_t *)hashtable)[0];
        printf("NB_BUCKETS: %d\n", nbuckets);
        uint32_t symoffset = ((uint32_t *)hashtable)[1];
        uint32_t bloom_size = ((uint32_t *)hashtable)[2];
        uint32_t bloom_shift = ((uint32_t *)hashtable)[3];
        elf_addr *bloom = (void *)&(hashtable[4]);
        uint32_t *buckets = (void *)&bloom[bloom_size];
        uint32_t *chain = &buckets[nbuckets];
        elf_addr word = bloom[(hash / (sizeof(elf_addr) * 8)) % bloom_size];
        elf_addr mask = 0 | (elf_addr)1 << (hash % (sizeof(elf_addr) * 8))
            | (elf_addr)1 << ((hash >> bloom_shift) % (sizeof(elf_addr) * 8));

        /*if ((word & mask) != mask)
            continue;*/
        uint32_t symix = buckets[hash % nbuckets];
        /*if (symix < symoffset)
            continue;*/

        while (1)
        {
            //char * symname = strtab + dynstr[symix].st_name;
            char * symname = (char *)dynstr + symix;
            printf("SYMNAME: %s", symname);
            uint32_t hashing = chain[symix - symoffset];
            if ((hash | 1) == (hashing | 1) && (strcmp(name, symname)) == 0)
            {
                res = &dynstr[symix] + map->l_addr;
                break;
            }
            if (hashing & 1)
                break;

            symix++;
        }
    }
    return res;
}
