#define GETPID 5
#define FORK  6
#define TPRINTF  7
static inline unsigned long  syscall(int num, unsigned long p0, unsigned long p1, unsigned long p2, unsigned long p3, unsigned long p4)
{
	unsigned long ret;
	asm volatile ( "int $20"
		: "=a" (ret)
		: "a" (num), "b" (p0), "c"(p1), "d" (p2), "S" (p3), "D" (p4)
	);
	return ret;
}

int open(const char *pathname, int flags, unsigned long mode)
{
	return syscall(1, (unsigned long)pathname, (unsigned long)flags, (unsigned long)mode, 0, 0);	
}

int read(int fd, void *buf, unsigned int count)
{
	return syscall(2, (unsigned long)fd, (unsigned long)buf, (unsigned long)count, 0, 0);
}

int write(int fd, const void *buf, unsigned int count)
{
	return syscall(3, (unsigned long)fd, (unsigned long)buf, (unsigned long)count, 0, 0);
}
int lseek(int fd, int offset, int whence) 
{
	return syscall(14,(unsigned long)fd, (unsigned long)offset, (unsigned long)whence, 0, 0);
}
int close(int fd) 
{
	return syscall(4, 0 , 0 , 0 , 0, 0);
}

int getpid()
{
	return syscall(5, 0 , 0 , 0 , 0, 0);
}

int fork()
{
	return syscall(6, 0 ,0 ,0 ,0, 0);	
}

int tprintf()
{

	return syscall(7, 0 ,0 ,0 ,0, 0);
}

int yield()
{

	return syscall(8, 0 ,0 ,0 ,0, 0);
}

int user_exit()
{
		return syscall(9, 0 ,0 ,0 ,0, 0);
}
int execve(const char *filename, char *const argv[],
                  char *const envp[]) {
	return syscall(10, (unsigned long)filename , (unsigned long)argv, (unsigned long)envp, 0, 0);
}

int opencons()
{
	return  syscall(11, 0 ,0 ,0 ,0, 0);
}

/* pid type */
int waitpid(int pid)
{
	return syscall(12, (unsigned long)pid, 0, 0, 0, 0);
}


/* pid type */
int alloc_vm_mem(unsigned long phys)
{
	return syscall(13, (unsigned long)phys, 0, 0, 0, 0);
}

int create_vcpu()
{
	return syscall(15, 0, 0, 0, 0, 0);
}

int run_vm()
{
	return syscall(16, 0, 0, 0, 0, 0);	
}

int mmap_mem(void *addr,  unsigned long length, void *gpa, int prot)
{
	return syscall(17, (unsigned long)addr, (unsigned long )length, (unsigned long)gpa, (unsigned long)prot, 0);
}
