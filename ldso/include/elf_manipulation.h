#ifndef ELF_MANIPULATION_H
#define ELF_MANIPULATION_H

#include <elf.h>
#include <link.h>
#include <sys/auxv.h>

struct link_map *full_map;

typedef ElfW(Ehdr) elf_ehdr;
typedef ElfW(Phdr) elf_phdr;
typedef ElfW(Shdr) elf_shdr;
typedef ElfW(Addr) elf_addr;
typedef ElfW(Sym) elf_sym;
typedef ElfW(Dyn) elf_dyn;
typedef ElfW(Off) elf_off;
typedef ElfW(auxv_t) elf_auxv_t;
typedef ElfW(Rela) elf_rela;
typedef ElfW(Xword) elf_xword;

/*struct elf_file {
    elf_addr vaddr;
    struct elf_ehdr *ehdr;
    struct elf_shdr *shdr;
};*/


elf_ehdr *get_elf_ehdr(char *filename);
elf_phdr *get_program_header(elf_ehdr *elf, char *name);
elf_shdr *get_section_header(elf_ehdr *elf, char *name);
elf_dyn *get_dynamic_section(elf_ehdr *elf, char *name);
char *get_dynamic_name(elf_xword val, char *name);
elf_sym *get_dynamic_element(elf_ehdr *elf, char *name, char *elt);
char *name_from_dynsim_index(elf_ehdr *elf,  char *name, int index);
int get_nb_rela(elf_ehdr *elf, char *name);
int get_nb_reladyn(elf_ehdr *elf, char *name);
int get_dynsym_size(elf_ehdr *elf, char *name);


#endif /*!ELF_MANIPULATION_H*/
