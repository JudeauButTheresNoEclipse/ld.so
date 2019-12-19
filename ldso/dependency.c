#include "include/dependency.h"
#include "include/elf_manipulation.h"
#include "include/elf_manipulation.h"
#include "string.h"
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "include/utility.h"

#define MAX_DEP 100


char *get_env(char *name);
elf_auxv_t *get_vdso(void);

static int is_in(char **res, char *name, int nb_dep)
{
    int ret = 0;
    for (int i = 0; i < nb_dep; i++)
        if (!strcmp(res[i], name))
            ret = 1;
    return ret;
}


static char *concatenate(char *path, char *name)
{
        char *ret = calloc(strlen(path) + strlen(name) + 10, 1);
        memcpy(ret, path, strlen(path));
        if (ret[strlen(ret) - 1] != '/')
            strcat(ret, "/");
        strcat(ret, name);
        return ret;
}


static char *test(char *path, char *name)
{
    path = strdup(path);
    char *buff = strtok(path, ":; ");
    while (buff)
    {
        char *ret = concatenate(buff, name);
        int o = open(ret, O_RDONLY);
        if (o < 0)
        {
            free(ret);
            buff = strtok(NULL, ":; ");
            if (buff)
                puts(buff);
            continue;
        }
        close(o);
        return ret;
    }
    return NULL;
}

static char *get_lib_absolute_path(char *name, char *binary)
{
    if (name[0] == '/')
        return name;
    if (test(".", name))
        return name;
    elf_ehdr *elf = get_elf_ehdr(binary);
    elf_dyn *dyn = get_dynamic_section(elf, binary);
    elf_dyn *cpy = dyn;
    char *path = NULL;
    char *res = NULL;
    char *string = (void *)get_dynamic_element(elf, binary, ".dynstr");
    
    while (dyn->d_tag != DT_NULL)
    {
        if (dyn->d_tag == DT_RPATH)
        {
            path = string + dyn->d_un.d_val;
            if ((res = test(path, name)) != NULL)
                goto out;
        }
        dyn++;
    }

    path = get_env("LD_LIBRARY_PATH");
    if (path && (res = test(path, name)) != NULL)
        goto out;
    
    dyn = cpy;
    while (dyn->d_tag != DT_NULL)
    {
        if (dyn->d_tag == DT_RUNPATH)
        {
            path = string + dyn->d_un.d_val;
            if ((res = test(path, name)) != NULL)
                goto out;
        }
        dyn++;
    }
    
    if ((res = test("/lib/", name)) != NULL)
        goto out;
    
    if ((res = test("/usr/lib/", name)) != NULL)
        goto out;
    
    else
    {
        printf("could not find library %s needed by %s in the system", name, binary);
        _exit(1);
    }

out:
    free(cpy);
    free(elf);
    return res;
}


static void fill_dependency(char **res, int *nb_dep)
{
    for (int i = 0; i < *nb_dep; i++)
    {   
        elf_ehdr *elf = get_elf_ehdr(res[i]);
        elf_dyn *dynamic = get_dynamic_section(elf, res[i]);
        while (dynamic->d_tag != DT_NULL)
        {
            if (dynamic->d_tag == DT_NEEDED)
            {
                char *name = get_dynamic_name(dynamic->d_un.d_val, res[i]);
                name = get_lib_absolute_path(name, res[i]);
                if (!is_in(res, name, *nb_dep))
                    res[(*nb_dep)++] = name;
            }
            dynamic++;
        }
        free(elf);
    }
}

char **build_dependency_table(char *executable_name)
{
    char **res = malloc(MAX_DEP * sizeof(char *));
    int nb_dep = 2;
    res[0] = executable_name;
    res[1] = "ld.so";
    char *preload = get_env("LD_PRELOAD");
    if (preload)
    {
        char *buff = strtok(preload, ":; ");
        do{
            res[nb_dep++] = buff;
            buff = strtok(NULL, ":; ");
        } while (buff);
    }
    int cpy = 0;
    do {
        cpy = nb_dep;
        fill_dependency(res, &nb_dep);
    } while(cpy != nb_dep);
    if (get_vdso())
        res[nb_dep++] = "linux-vdso.so.1";
    res[nb_dep] = NULL;
    return res;
}
