#ifndef DEPENDENCY_H
#define DEPENDENCY_H

#include "elf_manipulation.h"

char **build_dependency_table(char *executable_name, char **envp, elf_auxv_t *vdso);
char *get_lib_absolute_path(char *name, char *binary);
#endif /* DEPENDENCY_H */
