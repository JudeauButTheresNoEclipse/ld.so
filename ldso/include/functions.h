#ifndef FUNCTION_H
#define FUNCTION_H

#include <link.h>
#include <elf.h>
#include "elf_manipulation.h"

elf_addr load_program(elf_phdr *program, elf_ehdr *elf, struct link_map *map, elf_addr interp);
elf_addr load_elf_binary(struct link_map *map, elf_addr base);
struct link_map *build_link_map(char **table, elf_addr base, elf_addr vdso);
void resolve_relocations(struct link_map *next, struct link_map *map);

#endif /* !FUNCTION_H */
