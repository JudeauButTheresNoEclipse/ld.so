#ifndef RELOCATIONS_H
#define RELOCATIONS_H

#include <link.h>

void resolve_relocations(struct link_map *next, struct link_map *map, int lazy);
elf_addr runtime_relocations(struct link_map *next, int index);
void __reloc(void);

#endif /* !RELOCATIONS_H */
