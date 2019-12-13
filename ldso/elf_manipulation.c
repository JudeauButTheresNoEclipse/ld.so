#include "elf_manipulation.h"
#include "stdlib.h"
#include "string.h"



elf_rela *get_relocations(elf_ehdr *elf);
int get_nb_rela(elf_ehdr *elf);
char *name_from_dynsim_index(elf_ehdr *elf,  int index);
elf_ehdr *get_elf_ehdr(char *filename);
elf_phdr *get_program_header(elf_ehdr *elf);
elf_dyn *get_dynamic_section(elf);
char *get_dynamic_name(elf_xword *val);        
elf_sym *load_dynsym_section(elf_ehdr *elf);
elf_sym *link_map_lookup(struct link_map *map, char *rela_name);
