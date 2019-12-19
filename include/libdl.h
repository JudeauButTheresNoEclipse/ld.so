#ifndef LIBDL_H
#define LIBDL_H


typedef struct {
    const char *dli_fname;
    void *dli_fbase;
    const char *dli_sname;
    void *dli_saddr;
 } Dl_info;


void *dlopen(const char *filename, int flags);
int dlinfo(void *handle, int request, void *info);
void *dlvsym(void *handle, char *symbol, char *version);
int dlclose(void *handle);
int dladdr(void *addr, Dl_info *info);

#endif /* !LIBDL_H */
