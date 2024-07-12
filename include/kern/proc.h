#ifndef _PROC_H_
#define _PROC_H_
#include <kern/arch.h>

#define  MAXPROC 10
#define MAXOPENFILE  32

enum PROC_STATUS {
	RUNNING = 0,
	RUNNABLE,
	SLEEP,
	STOP
};

struct proc {
	struct pt_regs pregs;
	unsigned int pid;	
	unsigned int ppid;
	int status;
	struct proc *link;
	void *pgdir;
	struct file *fdtable[MAXOPENFILE];
};


void proc_init();
void schedule();
void memmap(unsigned int *pgdir, void *addr, unsigned int size);
#endif
