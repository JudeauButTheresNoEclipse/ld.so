#include "include/dependency.h"
#include "include/elf_manipulation.h"
#include "include/elf_manipulation.h"
#include "string.h"
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"


#define MAX_DEP 100
char *get_env(char *name);

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
        char *ret = malloc(strlen(path) + strlen(name) + 1);
        memcpy(ret, path, strlen(path));
        if (ret[strlen(ret) - 1] != '/')
            strcat(ret, "/");
        strcat(ret, name);
        return ret;
}


static int test(char *path, char *name)
{
    char *ret = concatenate(path, name);
    int o = open(ret, 0);
    if (o == -1)
    {
        free(ret);
        return 0;
    }
    close(o);
    puts(ret);
    free(ret);
    return 1;
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
    char *ret = NULL;
    char *string = (void *)get_dynamic_element(elf, binary, ".dynstr");
    while (dyn->d_tag != DT_NULL)
    {
        if (dyn->d_tag == DT_RPATH)
        {
            path = string + dyn->d_un.d_val;
            goto out;
        }
        dyn++;
    }
    path = get_env("LD_LIBRARY_PATH");
    if (path && test(path, name))
        goto out;
    dyn = cpy;
    while (dyn->d_tag != DT_NULL)
    {
        if (dyn->d_tag == DT_RUNPATH)
        {
            path = string + dyn->d_un.d_val;
            if (test(path, name))
                goto out;
        }
        dyn++;
    }
    if (test("/lib/", name))
    {
        path = "/lib/";
        goto out;

    }
    if (test("/usr/lib", name))
    {
        path = "/usr/lib";
        goto out;
    }
    else
    {
        printf("could not find library %s needed by %s in the system", name, binary);
        _exit(1);
    }
out:
    ret = concatenate(path, name);
    free(cpy);
    free(elf);
    return ret;
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
        free(dynamic);
    }
}

char **build_dependency_table(char *executable_name)
{
    char **res = malloc(MAX_DEP * sizeof(char *));
    int nb_dep = 1;
    res[0] = executable_name;
    int cpy = 0;
    do {
        cpy = nb_dep;
        fill_dependency(res, &nb_dep);
    } while(cpy != nb_dep);
    //res[nb_dep++] = "linux-vdso.so.1";
    res[nb_dep] = NULL;
    return res;
}
