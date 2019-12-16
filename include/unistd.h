#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>
#include <asm-generic/fcntl.h>
#include <linux/stat.h>
#include <bits/types/struct_timeval.h>
#include "syscall.h"
#include "types.h"

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

void _exit(int rc);
i64 write(int fd, const void *buf, size_t len);
i64 read(int fd, void *buf, size_t len);
int close(int fd);
int statx(int dirfd, const char *pathname, int flags,
                 unsigned int mask, struct statx *statxbuf);
struct iovec;

u64 lseek(int fd, u64 offset, int whence);
i64 writev(int fd, const struct iovec *iov, int iovcnt);
int gettimeofday(struct timeval *restrict tp, void *restrict tzp);

int open(const char *file, int flags, ...);


#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

void *mmap(void *addr, size_t len, int prot, int flags, int fd, i64 offset);
int munmap(void *addr, size_t len);
void *mremap(void *old_address, size_t old_size, size_t new_size, int flags, ...);

#endif /* !UNISTD_H */
