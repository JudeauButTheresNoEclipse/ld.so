#ifndef LIBDL_H
#define LIBDL_H

/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY	0x00001	/* Lazy function call binding.  */
#define RTLD_NOW	0x00002	/* Immediate function call binding.  */
#define	RTLD_BINDING_MASK   0x3	/* Mask of binding time value.  */
#define RTLD_NOLOAD	0x00004	/* Do not load the object.  */
#define RTLD_DEEPBIND	0x00008	/* Use deep binding.  */
/* If the following bit is set in the MODE argument to `dlopen',
   the symbols of the loaded object and its dependencies are made
   visible as if the object were linked directly into the program.  */
#define RTLD_GLOBAL	0x00100
#define RTLD_DI_LINKMAP	2
#define RTLD_DI_ORIGIN 6

/* Unix98 demands the following flag which is the inverse to RTLD_GLOBAL.
   The implementation does this by default and so we can define the
   value to zero.  */
#define RTLD_LOCAL	0

/* Do not delete object when closed.  */
#define RTLD_NODELETE	0x01000


typedef struct {
    const char *dli_fname;
    void *dli_fbase;
    const char *dli_sname;
    void *dli_saddr;
 } Dl_info;
#include "include/elf_manipulation.h"

extern void *_dlopen(char *filename, int flags);
extern int _dlinfo(void *handle, int request, void *info);
extern void *_dlsym(void *handle, char *symbol);
extern int _dlclose(void *handle);
extern int _dladdr(void *addr, Dl_info *info);

#endif /* !LIBDL_H */
