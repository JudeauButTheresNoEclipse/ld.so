#include "link_map.h"
#include "stdlib.h"
#include "string.h"
#include "readelf/header_info.h"

#include "stdio.h"

#include <elf.h>
#include <link.h>

struct link_map *link_map_add(struct link_map *map, char *name, ElfW(Ehdr) *elf)
{
    for (struct link_map *next = map; next; next = next->l_next)
        if (!strcmp(name, next->l_name))
            return NULL;
    struct link_map *new = malloc(sizeof(struct link_map));
    new->l_addr = (ElfW(Addr))elf; 
    new->l_name = name;
    new->l_ld = get_dynamic_section();
    new->l_next = NULL;
    
    if (!map)
    {
        new->l_prev = NULL;
        return new;
    }
    
    struct link_map *first = map;
    while (map->l_next)
        map = map->l_next;
    map->l_next = new;
    new->l_prev = map;

    return first;
}
