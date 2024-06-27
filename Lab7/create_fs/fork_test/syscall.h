#ifndef SYSCALL_INTERNAL_H
#define SYSCALL_INTERNAL_H

#define SYS_GETPID 0
#define SYS_UART_READ 1
#define SYS_UART_WRITE 2
#define SYS_EXEC 3
#define SYS_FORK 4
#define SYS_EXIT 5
#define SYS_MBOX_CALL 6
#define SYS_KILL 7
#define SYS_SIGNAL 8
#define SYS_POSIX_KILL 9

#ifndef __ASSEMBLER__

/* for user program */
void fork_test();
extern int getpid();
extern unsigned int uart_read(char buf[], unsigned int size);
extern unsigned int uart_write(char buf[], unsigned int size);
extern int exec(char *program_name, const char *argv[]);
extern void exit();
extern int fork();
extern void kill(int pid);
extern int mbox_call(unsigned char ch, unsigned int *mbox);

#endif

#endif