#ifndef LOADER_H
#define LOADER_H

#include <link.h>
#include <elf.h>
#include "elf_manipulation.h"

struct r_debug *r_debug;

elf_addr load_program(elf_phdr *program, elf_ehdr *elf, struct link_map *map);
elf_addr load_elf_binary(struct link_map *map);
struct link_map *build_link_map(char **table, elf_addr base, elf_addr vdso, struct link_map *map);
void _debug(void);

#endif /* !LOADER_H */
