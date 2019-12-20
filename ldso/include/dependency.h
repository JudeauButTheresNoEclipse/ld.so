#ifndef DEPENDENCY_H
#define DEPENDENCY_H


char **build_dependency_table(char *executable_name);
char *get_lib_absolute_path(char *name, char *binary);
#endif /* DEPENDENCY_H */
