#ifndef FUNCTION_H
#define FUNCTION_H

#include <link.h>
#include <elf.h>
#include "elf_manipulation.h"

char **build_dependency_table(char *executable_name);
elf_rela **build_relocation_table(char **dependency_table);
int load_program(elf_ehdr *elf);
int load_elf_binary(char *name, elf_auxv_t *auxv);


#endif /* !FUNCTION_H */
