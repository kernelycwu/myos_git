#include <kern/arch.h>
#include <kern/mm.h>
#include "ept.h"

int mmap_mem(unsigned long *pgdir, void *addr, unsigned long length, void *gpa,  unsigned int prot)
{
	unsigned long start;
	unsigned long sz;
        int i,r;
	struct page *pp;
        start = (unsigned long)addr / PAGESIZE * PAGESIZE;
        sz = (length + PAGESIZE - 1) / PAGESIZE * PAGESIZE;
	//gpa = (unsigned long)gpa / PAGESIZE * PAGESIZE;
		r = insert_page(pgdir, start + i, pp , VMX_EPT_PTE_FULL);

	return 0;
}
