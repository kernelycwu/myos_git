#include <kern/proc.h>

extern struct proc *current;

extern int sys_fork();
extern int sys_exit();
extern int sys_execve(const char *filename, char *const argv[],
                  char *const envp[]);

extern int sys_waitpid(int pid);
extern int sys_create_vcpus();
extern int sys_mmap_mem(void *addr, unsigned long length, void *gpa, int prot);

static int sys_yield()
{
	//print("-");
	//print(itoa(current->pid, 16));
	//print("-");
	schedule();
	return 0;
}

static int sys_tprintf()
{
	print("i@@@@@@@@@@@@@@@@@@:%x\n",current->pid);
	return 0;
}


int do_syscall(unsigned int num, unsigned long p0, unsigned long p1,
			 unsigned long p2,unsigned long p3,unsigned long p4)
{

	switch(num) {

		case 1:
			return sys_open(p0, p1, p2);
		case 2:
			return sys_read(p0, p1, p2);
		case 3:
			return sys_write(p0, p1, p2);
		case 4:
			break;

		case 6:
			return sys_fork();

		case 7:
			return sys_tprintf();

		case 8:
			return sys_yield();

		case 9:
			return sys_exit();

		case 10: 
			return sys_execve((const char *)p0, (char *const *)p1, (char *const *)p2);

		case 11:	
			return sys_opencons();
		case 12:
			return sys_waitpid(p0);
		case 13:
			return sys_alloc_vm_mem(p0);
		case 14:
			return sys_lseek((int)p0, (int)p1, (int)p2);
		case 15:
			return sys_create_vcpu();
		case 16:
			return sys_run_vm();
		case 17:
			return sys_mmap_mem((void *)p0, p1, (void*)p2, (int)p3);
		default:
			break;
	}
	return 0;
}
