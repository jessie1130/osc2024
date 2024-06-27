#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "exception.h"
#include "stddef.h"

int getpid(trapframe_t *tp);
size_t uartread(trapframe_t *tp, char buf[], size_t size);
size_t uartwrite(trapframe_t *tp, const char buf[], size_t size);
int exec(trapframe_t *tp, const char *name, char *const argv[]);
int fork(trapframe_t *tp);
void exit(trapframe_t *tp, int status);
int syscall_mbox_call(trapframe_t *tp, unsigned char ch, unsigned int *mbox);
void kill(trapframe_t *tp, int pid);
void register_signal(int SIGNAL, void (*handler)());
void signal_kill(int pid, int SIGNAL);
void* mmap(trapframe_t *tp, void *addr, size_t len, int prot, int flags, int fd, int file_offset);
void sigreturn(trapframe_t *tp);
int    open(trapframe_t *tp, const char *pathname, int flags);
int    close(trapframe_t *tp, int fd);
long   write(trapframe_t *tp, int fd, const void *buf, unsigned long count);
long   read(trapframe_t *tp, int fd, void *buf, unsigned long count);
int    mkdir(trapframe_t *tp, const char *pathname, unsigned mode);
int    mount(trapframe_t *tp, const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
int    chdir(trapframe_t *tp, const char *path);
long   lseek64(trapframe_t *tp, int fd, long offset, int whence);
int    ioctl(trapframe_t *tp, int fd, unsigned long request, void *info);
unsigned int get_file_size(char *thefilepath);
char        *get_file_start(char *thefilepath);

#endif /* _SYSCALL_H_*/