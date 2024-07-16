#ifndef __SYSCALLS_H
#define __SYSCALLS_H
extern int sys_fork();
extern int sys_exit();
extern int sys_execve(const char *filename, char *const argv[],
                  char *const envp[]);

extern int sys_waitpid(int pid);
extern int sys_create_vcpus();
extern int sys_mmap_mem(void *addr, unsigned long length, void *gpa, int prot);

extern int sys_open(const char *fname, int flags, int mode);
extern int sys_read(int fd, void *buf, unsigned int count);
extern int sys_write(int fd, void *buf, unsigned int count);
extern int sys_lseek(int fd, int offset, int whence);
extern int sys_close(int fd);
#endif
