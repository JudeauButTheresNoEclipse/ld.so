#ifndef LINK_MAP_H
#define LINK_MAP_H
#include <link.h>
#include <elf.h>

struct link_map *link_map_add(struct link_map *map, char *name,ElfW(auxv_t) *auxv);

#endif /* !LINK_MAP_H */
