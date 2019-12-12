#ifndef LDSO_H
#define LDSO_H
#include "types.h"

ElfW(auxv_t) * get_auxv_entry(ElfW(auxv_t) * auxv, u32 type);

#endif /* !LDSO_H */
