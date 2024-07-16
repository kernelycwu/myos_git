#include <kern/proc.h>
#include <kern/syscalls.h>
#include <syscallnr.h>
extern struct proc *current;

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
	print("current process pid:[%x]\n",current->pid);
	return 0;
}


int do_syscall(unsigned int num, unsigned long p0, unsigned long p1,
			 unsigned long p2,unsigned long p3,unsigned long p4)
{

	switch(num) {

		case __OPEN:
			return sys_open(p0, p1, p2);

		case __READ:
			return sys_read(p0, p1, p2);

		case __WRITE:
			return sys_write(p0, p1, p2);

		case __FORK:
			return sys_fork();

		case __YIELD:
			return sys_yield();

		case __EXIT:
			return sys_exit();

		case __EXECVE: 
			return sys_execve((const char *)p0, (char *const *)p1, (char *const *)p2);

		case __WAITPID:
			return sys_waitpid(p0);

		case __LSEEK:
			return sys_lseek((int)p0, (int)p1, (int)p2);

		case __TPRINTF:
			return sys_tprintf();

		case __OPENCONS:	
			return sys_opencons();

		case __ALLOC_VM_MEM:
			return sys_alloc_vm_mem(p0);

		case __CREATE_VCPU:
			return sys_create_vcpu();

		case __RUN_VM:
			return sys_run_vm();

		case __MMAP_MEM:
			return sys_mmap_mem((void *)p0, p1, (void*)p2, (int)p3);

		default:
			break;
	}
	return 0;
}
