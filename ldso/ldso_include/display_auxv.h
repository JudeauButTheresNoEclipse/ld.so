#ifndef DISPLAY_AUXV_H
#define DISPLAY_AUXV_H

#include <elf.h>
#include <link.h>

char *get_env_value(char **envp, char *name);
void print_env_values(char **envp);
void print_auxvp(ElfW(auxv_t) * auxv);
#endif /*!DISPLAY_AUXV_H*/
