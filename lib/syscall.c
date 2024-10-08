#include <syscallnr.h>
static inline 
unsigned long  syscall(int num, unsigned long p0, unsigned long p1, unsigned long p2, unsigned long p3, unsigned long p4)
{
	unsigned long ret;
	asm volatile ( "int $20"
		: "=a" (ret)
		: "a" (num), "b" (p0), "c"(p1), "d" (p2), "S" (p3), "D" (p4)
	);
	return ret;
}

/* File System */
int open(const char *pathname, int flags, unsigned long mode)
{
	return syscall(__OPEN, (unsigned long)pathname, (unsigned long)flags, (unsigned long)mode, 0, 0);	
}

int read(int fd, void *buf, unsigned int count)
{
	return syscall(__READ, (unsigned long)fd, (unsigned long)buf, (unsigned long)count, 0, 0);
}

int write(int fd, const void *buf, unsigned int count)
{
	return syscall(__WRITE, (unsigned long)fd, (unsigned long)buf, (unsigned long)count, 0, 0);
}

int lseek(int fd, int offset, int whence) 
{
	return syscall(__LSEEK,(unsigned long)fd, (unsigned long)offset, (unsigned long)whence, 0, 0);
}

int close(int fd) 
{
	return syscall(__CLOSE, 0 , 0 , 0 , 0, 0);
}

/* Process */
int getpid()
{
	return syscall(__GETPID, 0 , 0 , 0 , 0, 0);
}

int fork()
{
	return syscall(__FORK, 0 ,0 ,0 ,0, 0);	
}

int tprintf()
{

	return syscall(__TPRINTF, 0 ,0 ,0 ,0, 0);
}

int yield()
{

	return syscall(__YIELD, 0 ,0 ,0 ,0, 0);
}

int user_exit()
{
	return syscall(__EXIT, 0 ,0 ,0 ,0, 0);
}

int execve(const char *filename, char *const argv[],
		char *const envp[]) {
	return syscall(__EXECVE, (unsigned long)filename , (unsigned long)argv, (unsigned long)envp, 0, 0);
}

int waitpid(int pid)
{
	return syscall(__WAITPID, (unsigned long)pid, 0, 0, 0, 0);
}

int opencons()
{
	return  syscall(__OPENCONS, 0 ,0 ,0 ,0, 0);
}

/* Virtual Machine */
int alloc_vm_mem(unsigned long phys)
{
	return syscall(__ALLOC_VM_MEM, (unsigned long)phys, 0, 0, 0, 0);
}

int create_vcpu()
{
	return syscall(__CREATE_VCPU, 0, 0, 0, 0, 0);
}

int run_vm()
{
	return syscall(__RUN_VM, 0, 0, 0, 0, 0);	
}

int mmap_mem(void *addr,  unsigned long length, void *gpa, int prot)
{
	return syscall(__MMAP_MEM, (unsigned long)addr, (unsigned long )length, (unsigned long)gpa, (unsigned long)prot, 0);
}
