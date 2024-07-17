#include <elf.h>
#include <kern/proc.h>
#include <kern/lib.h>
#include <kern/mm.h>
#include <kern/arch.h>
#include <kern/file.h>

#define PTE_COW		0x800	
struct proc *procs = NULL;
struct proc *current = NULL;
unsigned char binary[128];

#ifdef amd64
	extern void* kpgdir;
#else
	extern unsigned char kpgdir[];
#endif

extern void *gdtdesc;
/* No head node free single list */
struct proc *proc_head = NULL;
/* init procs free list */
void 
init_proc()
{
	int i =0;
	struct proc *p = NULL;

	for(i = MAXPROC - 1; i >= 0; i--) {
		procs[i].link = p;
		p = &procs[i];
	}
	proc_head = p;

	return;
}

/* delete from procs free list */
struct proc *
alloc_proc()
{
	struct proc *p = NULL;

	p = proc_head;
	proc_head = proc_head->link;
	memset(p, 0 ,sizeof(struct proc));
	p->status = STOP;
	p->pid = p - &procs[0];
	p->link = NULL;

	return p;

}

/* add in  procs free list */
void 
free_proc(struct proc *p)
{
	p->link = proc_head;
	proc_head = p;
}

/* alloc process page table directory */
void *
alloc_vm(void *pgdir)
{
	struct page *pp = NULL;
	void *proc_pgdir = NULL;

	pp = alloc_page(1);
	if(!pp){
		print("alloc_vm err!");
		return pp;
	}
//	pp->count++;

	proc_pgdir = (void *)PAGE2VA(pp);	
	memcpy(proc_pgdir, pgdir, PAGESIZE);
	return proc_pgdir;	
}

void 
memmap(unsigned int *pgdir, void *addr, unsigned int size)
{
	unsigned int start;
	unsigned int sz;
	int i,r;
	struct page *pp = 0;

	start = (unsigned long)addr / PAGESIZE * PAGESIZE;
	sz = (size + PAGESIZE - 1) / PAGESIZE * PAGESIZE;
	for(i = 0; i < sz; i += PAGESIZE) {
		pp = alloc_page(0);
		//pp->count++;
                r = insert_page(pgdir, start + i, pp , PTE_U|PTE_W|PTE_P);
	}
	
}

int load_img(struct proc *p, unsigned char *binary, unsigned int size)
{
	Elf_Ehdr *ehdr;
        Elf_Phdr *phdrs, *phdr;
        int i, r;
        void *output = (void *)binary;
	struct page *pp = NULL;

        ehdr = (Elf_Ehdr *) output;
        if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
                        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
                        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
                        ehdr->e_ident[EI_MAG3] != ELFMAG3) {

                return;
        }
        phdrs = (Elf_Phdr *)(output + ehdr->e_phoff);

        for (i = 0; i < ehdr->e_phnum; i++) {
                phdr = &phdrs[i];
                switch (phdr->p_type) {
                        case PT_LOAD:
				  memmap(p->pgdir, (void *)phdr->p_vaddr, phdr->p_memsz);
                                  memcpy( (void *)phdr->p_vaddr, (void *)output+phdr->p_offset, phdr->p_memsz);
                                //
                                break;
                        default: /* Ignore other PT_* */ break;
                }
        }
	/* alloc prog stack */
	pp = alloc_page(1);
	//pp->count++;
	r = insert_page(p->pgdir, USTACKTOP - PAGESIZE, pp , PTE_U|PTE_W|PTE_P);
#ifndef amd64
	p->pregs.esp = USTACKTOP;
	p->pregs.eip =  ehdr->e_entry;
#else
	p->pregs.rsp = USTACKTOP;
	p->pregs.rip =  ehdr->e_entry;

#endif
}
#ifndef amd64
void run_proc(struct proc *p)
{
	current = p;
	asm volatile ("movl %0, %%cr3"
			:
			: "r" (p->pgdir - 0xc0000000)  );

	asm volatile ( "movl %0, %%esp \n\t"
			"popal \n\t"
			"popl %%es \n\t"
			"popl %%ds \n\t"
			"addl $8, %%esp\n\t"
			"iret"
			:
			: "g" ((void *)&p->pregs)
		     ); 

}
#else
static void print_regs1(struct pt_regs *pt)
{
        print("==========pt_reg begin============\n");
        print("r15     %lx\n", pt->r15);
        print("r14     %lx\n", pt->r14);
        print("r13     %lx\n", pt->r13);
        print("r12     %lx\n", pt->r12);
        print("rbp     %lx\n", pt->rbp);
        print("rbx     %lx\n", pt->rbx);
        print("r11     %lx\n", pt->r11);
        print("r10     %lx\n", pt->r10);
        print("r9      %lx\n", pt->r9);
        print("r8      %lx\n", pt->r8);
        print("rax     %lx\n", pt->rax);
        print("rcx     %lx\n", pt->rcx);
        print("rdx     %lx\n", pt->rdx);
        print("rsi     %lx\n", pt->rsi);
        print("rdi     %lx\n", pt->rdi);
        print("trapno  %lx\n", pt->num);
        print("errcode %lx\n", pt->err_code);
        print("eip     %lx\n", pt->rip);
        print("cs      %lx\n", pt->cs);
        print("eflags  %lx\n", pt->eflags);
        print("rsp     %lx\n", pt->rsp);
        print("ss      %lx\n", pt->ss);
        print("==========pt_reg end ============\n");
}
static __inline void
tlbflush(void)
{
        unsigned long cr3;
       	asm volatile("movq %%cr3,%0" : "=r" (cr3));
        asm volatile("movq %0,%%cr3" : : "r" (cr3));
}
static inline void lcr3(unsigned long cr3)
{
	asm volatile("movq %0, %%cr3" : : "r" (cr3));
}

void run_proc(struct proc *p)
{
/*	
	asm volatile ("movq %0, %%cr3"
			:
			: "r" (p->pgdir - KBASE)  );
*/
	lcr3((unsigned long)(p->pgdir - KBASE));
	current = p;
	asm volatile ( "movq %0, %%rsp \n\t"
			"popq %%r15 \n\t"
			"popq %%r14 \n\t"
			"popq %%r13 \n\t"
			"popq %%r12 \n\t"
			"popq %%rbp \n\t"
			"popq %%rbx \n\t"
			"popq %%r11 \n\t"
			"popq %%r10 \n\t"
			"popq %%r9 \n\t"
			"popq %%r8 \n\t"
			"popq %%rax \n\t" 
			"popq %%rcx \n\t"
			"popq %%rdx \n\t"
			"popq %%rsi \n\t"
			"popq %%rdi \n\t"
			"addq $16, %%rsp \n\t"
			"iretq \n\t"
			:
			: "g" ((void *)&p->pregs)
			: "memory"
		     ); 

}

#endif
int 
create_proc(unsigned char *binary, unsigned int size)
{
	struct proc *p = 0;
	int i;

	/* alloc process control block */
	p = alloc_proc();
	if(p == NULL) {
		panic("can't alloc proc structure\n");
		return -1;
	}
	p->pregs.cs = 0x1b;
#ifndef amd64
	p->pregs.ds = 0x23;
	p->pregs.es = 0x23;
#endif
	p->pregs.ss = 0x23;
	p->pregs.eflags |= 0x200;



	/* alloc virtual memory */
	p->pgdir = alloc_vm(kpgdir);

	/* alloc file resources */
	for(i = 0; i < MAXOPENFILE; i++) {
		p->fdtable[i] = NULL;
        }



#ifndef amd64

	asm volatile ("movl %0, %%cr3"
                        :
                        : "r" (p->pgdir - 0xc0000000)  ); 

	extern void *gdtdesc;
	asm volatile ("lgdt %0" : "=m"(gdtdesc));
#else
	lcr3((unsigned long)(p->pgdir - KBASE));
        asm volatile ("lgdt %0" : "=m"(gdtdesc));
#endif


	/* mapping execute binary file */
	load_img( p, binary, size);

	/*set process status */
	p->status = RUNNABLE;

	if(current == NULL )	
		current = p;
	run_proc(p);

	return 0;
}
extern unsigned char _binary_user_init_start[];
extern unsigned char _binary_user_init_end[];
extern unsigned int _binary_user_init_size;

//int 
void
spawn_proc()
{
//	create_proc(_binary_user_hello_start, _binary_user_hello_size);
	create_proc(_binary_user_init_start, _binary_user_init_end - _binary_user_init_start);
//		extern unsigned char _binary____user_world_start[];
//	extern unsigned int _binary____user_world_size;
//	create_proc(_binary____user_world_start, _binary____user_world_size);

//	return 0;
}

#ifndef amd64
static void destroy_vm(struct proc *p)
{
	unsigned int addr = 0;
        unsigned int start = UTEXT;
        unsigned int end = KBASE;
	unsigned int pa_addr;
	struct page *pp;
	unsigned int pde;
	unsigned int pgdindex;

	/* 1. free page */
	for(addr = start; addr < end; addr += PAGESIZE) {
                remove_page(p->pgdir, addr);
        }
	
	/* 2. free page table page */
	for(addr = start; addr < end; addr += PAGESIZE) {
		pgdindex = (unsigned int)addr >> 22;
		pde = ((unsigned int *)p->pgdir)[pgdindex];
		if ( pde & PTE_P) {
			pa_addr = pde & 0xfffff000;
			pp = PA2PAGE(pa_addr >> 12);
			free_page(pp);
		}
	}
	/* 3. free pgdir page */
	free_page(VA2PAGE((unsigned int )p->pgdir));
	p->pgdir = NULL;

}
#else
void free_pgtable(void *pgdir);
void * alloc_full_vm_new(void *pgdir);

static void destroy_vm(struct proc *p)
{
	unsigned long addr = 0;
        unsigned long start = UTEXT;
        unsigned long end = USTACKTOP;
	print("destroy_vmk:n");	
	/* 1. free page */
        for(addr = start; addr < end; addr += PAGESIZE) {
                remove_page(p->pgdir, addr);
        }
	/* 2. free page table page */
	free_pgtable(p->pgdir);

	/* 3. free pgdir page */
	free_page(VA2PAGE((unsigned long)p->pgdir));
	p->pgdir = NULL;

}

#endif
static void 
destroy_proc(struct proc *p)
{
	int i;
	if(p == NULL) return;
	print("destroy_proc\n");	
	/* release  memory resources */
  	destroy_vm(p);
#if 1
	/* release file resources */
        for(i = 0; i < MAXOPENFILE; i++) {
                if(p->fdtable[i] != NULL) {
                        p->fdtable[i]->refcount--;
			if(p->fdtable[i]->refcount == 0)
				p->fdtable[i] = NULL;
                }
        }
#endif	
	/* release process control block */
	free_proc(p);
	/* set process status */
	p->status = STOP;

	/* reschedule and run others */
	if(current == p) {
		current = NULL;
		schedule();
	}
}

void schedule()
{
	struct proc *p = 0;
	unsigned int curind = 0; 
	unsigned int nextind = 0;

	if(current == NULL ) {
		curind = 0;
	} else
		curind = current->pid;

	nextind = curind + 1;

	while( curind != nextind) {
		p = &procs[nextind];
		if(p->status == RUNNABLE ) {
			run_proc(p);
		}

		nextind++ ;
		nextind = nextind % MAXPROC;	
	}
	
	p = &procs[curind];
	if(p->status == RUNNABLE) {
		run_proc(p);
	}
	else {
		print("No runnable process\n");
		current = NULL;
		asm volatile("sti;hlt");
		//asm volatile("hlt");
	}
}

int sys_fork()
{
	struct proc *p = NULL;
	unsigned long addr = 0;
	unsigned long *pte;
	int r, i;
	struct page *pp;

	/* allocate process control block */
	p = alloc_proc();
	if(p == 0) {
		print("proc emplty");
		return -1;
	}

	/* copy cpu context */
	memcpy((void *)&p->pregs, (void *)&current->pregs, sizeof(struct pt_regs));

#ifndef amd64
	p->pregs.eax = 0;
#else
	p->pregs.rax = 0;
#endif


	/* copy address space */
	//p->pgdir = alloc_full_vm_new(current->pgdir);
	p->pgdir = alloc_vm(kpgdir);
	if(!p->pgdir) {
		print("alloc virtual address space failed\n");
		return -1;
	}
	//for(addr = UTEXT; addr < KBASE; addr += PAGESIZE) {
	for(addr = UTEXT; addr < USTACKTOP - PAGESIZE ; addr += PAGESIZE) {
	//for(addr = UTEXT; addr < USTACKTOP; addr += PAGESIZE) {
		if((r = walk_page(current->pgdir, addr, 0, &pte)) < 0)
			continue;
		if ( *pte == 0)
			continue;
		
		pp = (struct page *)PA2PAGE((*pte) >> 12);	
		r = insert_page(p->pgdir, addr, pp,  PTE_U|PTE_P|PTE_COW);
		if(r < 0)
			return r;

		pp->count++;
		
		*pte = ((*pte) / PAGESIZE * PAGESIZE) | PTE_U|PTE_P|PTE_COW;
		 tlb_invalid(addr);	
		//*pte = *pte & 0xFFFFF000 | PTE_U|PTE_P|PTE_COW;		
		//r = insert_page(current->pgdir, addr, pp,  PTE_U|PTE_P|PTE_COW);
	}

	/* alloc prog stack */
        pp = alloc_page(1);
	memcpy((void *)PAGE2VA(pp), (void *)(USTACKTOP - PAGESIZE), 4096); 

	r = insert_page(p->pgdir, USTACKTOP - PAGESIZE, pp , PTE_U|PTE_W|PTE_P);

	/* copy open files */
	for(i = 0; i < MAXOPENFILE; i++) {
		if(current->fdtable[i] != NULL) {
			p->fdtable[i] = current->fdtable[i];
			p->fdtable[i]->refcount++;
		}
	}

	/* set process status */
	p->status = RUNNABLE;
	return p->pid;
}



int sys_waitpid(int pid)
{	
	struct proc *p = NULL;

	if(pid > MAXPROC || pid < 0)
		return -1;
	p = &procs[pid];

	while(p->status != STOP) {
                schedule();
	}
	return pid;	
}

int sys_exit()
{
	print("sys_exit\n");
	destroy_proc(current);
	return 0;
}

#ifdef amd64

#define pml_index(address)  (((unsigned long)(address)) >> PML_SHIFT & 0x1ff)
#define pml_offset(pml, address) (((unsigned long*)(pml)) + pml_index((address)))

#define pgtable_page(addr) ((unsigned long*)KPA2VA((*(addr))/PAGESIZE * PAGESIZE))

#define pdp_index(addr)  (((unsigned long)(addr)) >> PDP_SHIFT & 0x1ff)
#define pdp_offset(pdp, addr) ((pgtable_page((pdp))) + pdp_index((addr)))


#define pgd_index(addr)  (((unsigned long)(addr)) >> PGD_SHIFT & 0x1ff)
#define pgd_offset(pgd, addr) ((pgtable_page((pgd))) + pgd_index((addr)))

#define pte_index(addr)  (((unsigned long)(addr)) >> PTE_SHIFT & 0x1ff)
#define pte_offset(pte, addr) ((pgtable_page((pte))) + pte_index((addr)))

#define PML_SIZE	(1UL<< PML_SHIFT)
#define PML_MASK	(~(PML_SIZE - 1))
#define PDP_SIZE	(1UL << PDP_SHIFT)
#define PDP_MASK	(~(PDP_SIZE - 1))
#define PGD_SIZE	(1UL << PGD_SHIFT)
#define PGD_MASK	(~(PGD_SIZE - 1))

void free_pgtable(void *pgdir)
{
	unsigned long *src, *src_pml, *src_pdp, *src_pgd, *src_pte;
	unsigned long addr = UTEXT, end = USTACKTOP;
	src = pgdir;
	do {
		src_pml = pml_offset(src, addr);	
		if(*src_pml == 0) goto pml_out;
		do {
			src_pdp = pdp_offset(src_pml, addr);
			if(*src_pdp == 0) goto pdp_out;
			do {
                                src_pgd = pgd_offset(src_pdp, addr);
                                if(*src_pgd == 0) goto pgd_out;
				print("pgd_free:%lx\n", *src_pgd);
				free_page(PA2PAGE(*src_pgd >> 12));
				*src_pgd = NULL;	
			pgd_out:
                                addr = (addr +  PGD_SIZE) & PGD_MASK;
                                if(addr >= end) {
                                        goto out_pdp_free;
				}
                                src_pgd++;
                        }while((unsigned long) src_pgd & (PAGESIZE-1));

			free_page(PA2PAGE(*src_pdp >> 12));
			print("pdp_free:%lx\n", *src_pdp);
			*src_pdp = NULL;	
		pdp_out:
			addr = (addr + PDP_SIZE) & PDP_MASK;
			if(addr >= end)
				goto out_pml_free;

			src_pdp++;

		} while((unsigned long)src_pdp & (PAGESIZE - 1));
		free_page(PA2PAGE(*src_pml >> 12));
		print("pml_free:%lx\n", *src_pml);
		*src_pml = NULL;
	pml_out:
		addr = (addr + PML_SIZE) & PML_MASK;
		if(addr >= end)
			goto out;

		src_pml++;
	} while((unsigned long)src_pml & (PAGESIZE - 1));
out:
        return ;

out_pdp_free:
	free_page(PA2PAGE(*src_pdp >> 12));
	print("pdp_free:%lx\n", *src_pdp);
	*src_pdp = NULL;
out_pml_free:
	free_page(PA2PAGE(*src_pml >> 12));
	print("pml_free:%lx\n", *src_pml);
	*src_pml = NULL;
	return;
}

void *
alloc_full_vm_new(void *pgdir)
{
	unsigned long *src, *src_pml, *src_pdp, *src_pgd, *src_pte;
	unsigned long *dst, *dst_pml, *dst_pdp, *dst_pgd, *dst_pte;
	unsigned long addr = UTEXT, end = USTACKTOP;
	struct page *pp = NULL;

	src = pgdir;
	pp = alloc_page(1);
        dst = (void *)PAGE2VA(pp);

	do {
		src_pml = pml_offset(src, addr);
		dst_pml = pml_offset(dst, addr);

		if(*src_pml == 0) goto pml_out;
		print("pml:%lx\n", *src_pml);
		print("pml_index:%lx\n", pml_index(addr));				
		if(*dst_pml == 0) {
			pp = alloc_page(1);
                	*dst_pml = PAGE2PA(pp) | PTE_U| PTE_W| PTE_P;
		}
		do {
			src_pdp = pdp_offset(src_pml, addr);
			dst_pdp = pdp_offset(dst_pml, addr);

			if(*src_pdp == 0) goto pdp_out;

			print("pdp_index:%lx\n", pdp_index(addr));				
			print("pdp:%lx\n", *src_pdp);

			if(*dst_pdp == 0) {
				pp = alloc_page(1);
				*dst_pdp = PAGE2PA(pp) | PTE_U| PTE_W| PTE_P;
			}

			do {
				src_pgd = pgd_offset(src_pdp, addr);
				dst_pgd = pgd_offset(dst_pdp, addr);

				if(*src_pgd == 0) goto pgd_out;
				print("pgd_index:%lx\n", pgd_index(addr));				
				print("pgd_addr:%lx\n", *src_pgd);				
			
			
				if(*dst_pgd == 0) {
					pp = alloc_page(1);
					*dst_pgd = PAGE2PA(pp) | PTE_U| PTE_W| PTE_P;
				}

				do {
					src_pte = pte_offset(src_pgd, addr);
					dst_pte = pte_offset(dst_pgd, addr);
				
					if(*src_pte == 0) goto pte_out;
					print("pte_index:%lx\n", pte_index(addr));	
					if(*dst_pte == 0) {
						*dst_pte = 0;					
					}
				 pte_out:
					addr = addr + PAGESIZE;
					if(addr >= end)
						goto out;
					src_pte++;
					dst_pte++;					
				} while((unsigned long )src_pte & (PAGESIZE-1));
			pgd_out:
				addr = (addr +  PGD_SIZE) & PGD_MASK;
				if(addr >= end)
					goto out;

				src_pgd++;
				dst_pgd++;
			}while((unsigned long) src_pgd & (PAGESIZE-1));
		pdp_out:
			addr = (addr + PDP_SIZE) & PDP_MASK;
			if(addr >= end)
				goto out;

			src_pdp++;
			dst_pdp++;

		} while((unsigned long)src_pdp & (PAGESIZE - 1));
	pml_out: 
		addr = (addr + PML_SIZE) & PML_MASK;
		if(addr >= end) 
			goto out;
		src_pml++;
		dst_pml++;
	} while((unsigned long)src_pml & (PAGESIZE - 1));
out:
	return dst;		
}
#endif
