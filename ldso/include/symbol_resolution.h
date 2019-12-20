#ifndef SYMBOL_RESOLUTION_H
#define SYMBOL_RESOLUTION_H

#include <link.h>
#include "elf_manipulation.h"


elf_addr link_map_lookup(struct link_map *map, char *rela_name);
elf_addr gnu_hash_lookup(struct link_map *next, char *rela_name);




#endif /*! SYMBOL_RESOLUTION_H */
