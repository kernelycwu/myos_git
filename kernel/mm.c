#include <kern/arch.h>
#include <kern/console.h>
#include <kern/mm.h>
#include <kern/proc.h>
#include <kern/lib.h>
/* https://bochs.sourceforge.io/techspec/CMOS-reference.txt */
/* https://wiki.osdev.org/Detecting_Memory_(x86) */
/*
 * ----------R30--------------------------------
 *  CMOS 30h - IBM - EXTENDED MEMORY IN KB (low byte)
 *  SeeAlso: CMOS 17h"IBM",CMOS 31h
 *  ----------R31--------------------------------
 *  CMOS 31h - IBM - EXTENDED MEMORY IN KB (high byte)
 *   (this appears to mirror the value in bytes 17h-18h.)
 *   SeeAlso: CMOS 18h"IBM",CMOS 30h
 */
#define NULL (0)
static unsigned int mem_size_kb = 0;
static unsigned int npages = 0;
struct page *pages = NULL; 
struct page *phead = NULL;
unsigned long kused_mem = 0;


#ifdef amd64
void *kpgdir = NULL;
extern unsigned char boot_pgkdir[];
#else
extern unsigned char kpgdir[];
#endif

unsigned short detect_memsize()
{
	unsigned short total;
	unsigned char lowmem, highmem;

	outb(0x70, 0x30);
	lowmem = inb(0x71);
	outb(0x70, 0x31);
	highmem = inb(0x71);

	total = lowmem | highmem << 8;
	return total;
}
void tlb_invalid(void *va)
{
	asm volatile (
		"invlpg	(%0)"
		:
		: "r" (va)
		: "memory"
	);
}

#define TRAMPOLINE_BASE 0x8000
void 
init_page(void)
{
	int i;
	struct page *p=NULL, *q = NULL;
	kused_mem -= KBASE;

	for(i = 0; i < npages; i++) {
	 	pages[i].count++;
		pages[i].link = NULL;
	}

	print("mem_size_kb:%lx\n", kused_mem);
        for(i = npages - 1; i >= (kused_mem + PAGESIZE -1) / PAGESIZE; i--) {
                pages[i].link = p;
                p = &pages[i];
                p->count = 0;
        }

/*
	for(i = 0x000A0000 / PAGESIZE -1; i  >= 1; i--) {
		pages[i].link = p;
		p = &pages[i];
		p->count = 0;
	}
*/
	for(i = TRAMPOLINE_BASE / PAGESIZE -1; i  >= 1; i--) {
                pages[i].link = p;
                p = &pages[i];
                p->count = 0;
        }

	
	for(i = 0x000A0000 / PAGESIZE - 1; i > TRAMPOLINE_BASE / PAGESIZE; i--) {
		pages[i].link = p;
		p = &pages[i];
		p->count = 0;
	}

	phead = p;
#if 0
	for(i = 1; i < 0x000A0000 / PAGESIZE; i++) {
		pages[i].link = phead;
		phead = &pages[i];
		pages[i].count = 0;
	} 

	i = (kused_mem + PAGESIZE -1) / PAGESIZE;
	for(; i < npages; i++) {
		pages[i].link = phead;
                phead = &pages[i];
                pages[i].count = 0;
	}
#endif
}

struct page * 
alloc_page(int flags)
{
	struct page * p = NULL;

	if( phead != NULL) {
		p = phead;
		phead = phead->link;
		if(flags & PGZERO)
			memset((void*)PAGE2VA(p), 0 , PAGESIZE);
		p->count++; 
	}

	return p;
}

void 
free_page(struct page *p)
{
	struct page *q = NULL;
	if(p  == NULL)
		return;
	p->count--; 
	if(p->count == 0 ) {
		print("free_page:%lx\n", p);
		p->link = phead;
		phead = p;
	}
}
#ifndef amd64
int 
walk_page(unsigned long *pgdir, void *va,  int create, unsigned long **pte)
{
	unsigned long *pde = NULL;
	struct page *p = NULL;
	unsigned long pa_addr;

	unsigned long pgdindex = (unsigned long)va >> 22;
	unsigned long pteindex = (unsigned long)va >> 12 & 0x3ff;

	if (pgdir[pgdindex] & PTE_P) {
		pa_addr = pgdir[pgdindex] & 0xfffff000;
		pde = (unsigned long *)KPA2VA(pa_addr);

		*pte = &pde[pteindex];

		return 0;                                       
	}	

	if(create == 1) {
		p = alloc_page(1); 
		//p->count++;

		pgdir[pgdindex] = PAGE2PA(p) | PTE_U| PTE_W| PTE_P;
		pde = (unsigned long *)PAGE2VA(p);

		*pte = &pde[pteindex];
		return 0;

	}
 
	*pte = 0;
	return -1;


}
#else
int 
walk_dir(unsigned long *pgdir, void *va,  int create, unsigned long **pte)
{
	unsigned long *pde = NULL;
	struct page *p = NULL;
	unsigned long pa_addr;

	unsigned long pgdindex = (unsigned long)va >> PGD_SHIFT & 0x1ff;
	//unsigned long pteindex = (unsigned long)va >> PTE_SHIFT & 0x3ff;
	unsigned long pteindex = (unsigned long)va >> PTE_SHIFT & 0x1ff;


	if (pgdir[pgdindex] & PTE_P) {
		pa_addr = (pgdir[pgdindex] / PAGESIZE)  * PAGESIZE;
		pde = (unsigned long *)KPA2VA(pa_addr);
		*pte = &pde[pteindex];
		
		//print("pde:%lx\n", pgdir[pgdindex]);
		return 0;                                       
	}	

	if(create == 1) {


		p = alloc_page(1); 
		//p->count++;

		//print("pgdindex:%lx\n", pgdindex);
		pgdir[pgdindex] = PAGE2PA(p) | PTE_U| PTE_W| PTE_P;
//		print("#pgd:%lx\n", pgdir[pgdindex]);

		pde = (unsigned long *)PAGE2VA(p);
		*pte = &pde[pteindex];
//		print("#pte:%lx\n", pde[pteindex]);
		return 0;

	}
 
	*pte = 0;
	return -1;


}

int
walk_page(unsigned long *pgdir, void *va,  int create, unsigned long **pte)
{
	unsigned long *pml4 = NULL;
	unsigned long *pdpt = NULL;
        struct page *p = NULL;
        unsigned long pa_addr;


        unsigned long pmlindex = (unsigned long)va >> PML_SHIFT & 0x1ff;
        unsigned long pdpindex = (unsigned long)va >> PDP_SHIFT & 0x1ff;	
	if (pgdir[pmlindex] & PTE_P) {
		pa_addr = (pgdir[pmlindex] / PAGESIZE)  * PAGESIZE ;
		pml4 = (unsigned long *)KPA2VA(pa_addr);
		// print("pml:%lx\n", pgdir[pmlindex]);
		if(pml4[pdpindex] & PTE_P) {
			pa_addr = (pml4[pdpindex] / PAGESIZE) * PAGESIZE;

		//	print("pdp:%lx\n", pml4[pdpindex]);			
			pdpt = (unsigned long *)KPA2VA(pa_addr);			
		} else {
			if(!create)  return -1;
			p = alloc_page(1);
			pml4[pdpindex] = PAGE2PA(p) | PTE_U| PTE_W| PTE_P;
			pdpt = (unsigned long *)PAGE2VA(p);				
//			print("pdp:%lx\n", pml4[pdpindex]);			

		}
			
	} else {

		if(!create) return -1;
		p = alloc_page(1);
		//p->count++;
		pgdir[pmlindex] = PAGE2PA(p) | PTE_U| PTE_W| PTE_P;
		pml4 = (unsigned long *)PAGE2VA(p);
		
		//print("\n#pml:%lx\n", pgdir[pmlindex]);
		p = alloc_page(1);
		pml4[pdpindex] = PAGE2PA(p) | PTE_U| PTE_W| PTE_P;
		//print("#pdp:%lx\n", pml4[pdpindex]);
		pdpt = (unsigned long *)PAGE2VA(p);
	}
	return walk_dir(pdpt, va , create, pte);
}
#endif
int insert_page(unsigned long *pgdir, void *va, struct page *pp,  int flag)
{
	int r;
	unsigned long *pte;
	if(pp == NULL) {
		print("insert_page error");
		return -1;
	}
	r = walk_page(pgdir, va, 1, &pte);
	if(r < 0 ) {
		print("walk_page error\n");
		return r;
	}
	//*pte = PAGE2PA(pp) | PTE_U|PTE_W|PTE_P;
	*pte = PAGE2PA(pp) | flag;
	//print("#pte:%lx\n", *pte);
	tlb_invalid(va);
	return 0;				
}

int remove_page(unsigned long *pgdir, void *va)
{
	int r;
	unsigned long i;
	unsigned long *pte;

	r = walk_page(pgdir, va, 0, &pte);
        if(r < 0 )
                return r;

	/*FIXME: free_page */
	if(*pte & PTE_P) {
		print("remove:%ld\n", *pte);
		//i = ((*pte) & 0xfffff000)/4096;	
		i = (*pte) >> PTE_SHIFT;
		free_page(&pages[i]);
	}
	
	*pte = 0;
	tlb_invalid(va);
	return 0;	
}

#if 1
static void
check_phead()
{
        struct  page *pp0, *pp1;
        int nfree_basemem = 0, nfree_extmem = 0;
        char *first_free_page;

        //if (!phead)

	pp0 = alloc_page(0);
	memset((void *)PAGE2VA(pp0), 1, PAGESIZE);


			//print("========walk_page!");
	//print(itoa(pte, 16));
	//print("========walk_page!");

	pp0 = alloc_page(0);
	print(itoa(PAGE2PA(pp0), 16));

unsigned long *pgdir = (unsigned long *)kpgdir;
	unsigned long *pte = 0;
	int r;


	r = insert_page(pgdir, (unsigned long *)0x8048000, pp0, PTE_U|PTE_W|PTE_P);
	 if(r < 0)
                print("insert_page!");


	r = walk_page(pgdir, (unsigned long *)0x8048000, 0, &pte);
	if(r < 0)
                print("walk_page!");

	print("i======%lx", *pte);


	int *p = (int *)0x8048000;
	*p = 0x123456;
	//memset((void *)0x8048000, 'g', PAGESIZE);
/*
	print(itoa(*p, 16));	

*/
	r = remove_page(pgdir, (unsigned long *)0x8048000);	
	if(r < 0)
		print("remove_page!");

//	free_page(pp0);


	 pp1 = alloc_page(0); 
        memset((void *)PAGE2VA(pp1), 1, PAGESIZE);
        free_page(pp1);
//	if(pp0 == pp1)
		print(itoa((unsigned long)pp0, 16));
		print(itoa((unsigned long)pp1, 16));
#if 0
#endif	
}
#endif
extern struct proc *procs;
unsigned char *mem_code;

	//0x0f, 0x01, 0xc1, /* vmcall */
const unsigned char code[] = { 
        0xba, 0xf8, 0x03, /* mov $0x3f8, %dx */
        0xb0, 'H',       /* mov $'H', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'e',       /* mov $'e', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'l',       /* mov $'l', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'l',       /* mov $'l', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'o',       /* mov $'o', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, ',',       /* mov $',', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, ' ',       /* mov $' ', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'w',       /* mov $'w', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'o',       /* mov $'o', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'r',       /* mov $'r', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'l',       /* mov $'l', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'd',       /* mov $'d', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, '!',       /* mov $'!', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, '\n',       /* mov $'\n', %al */
        0xee,             /* out %al, (%dx) */
        0xf4,             /* hlt */
    };
void init_mem(void)
{
	extern unsigned char _start[], _end[];

	mem_size_kb = detect_memsize();	

	npages = mem_size_kb << 2;
	kused_mem = (unsigned long)_end;

#ifdef amd64
	kused_mem = ((kused_mem + PAGESIZE - 1) / PAGESIZE) * PAGESIZE;
	kpgdir = (void *)kused_mem;
	/*kernel base 0xFFFFFFFF80200000*/
	((unsigned long *)kpgdir)[511] = ((unsigned long *)boot_pgkdir)[511]; 
	/*user base   0x0000000080000000*/
	//((unsigned long *)kpgdir)[0] = ((unsigned long *)boot_pgkdir)[0]; 
	kused_mem += PAGESIZE ;
#endif
/*
	extern void *vmxpde1;	
	mem_code = (unsigned char *)kused_mem;
	
	//memcpy(mem_code + 0x7000 , code, sizeof(code));
	//((unsigned long *)vmxpde1)[0] = (unsigned long)(kused_mem - KBASE + 0x87);
	//((unsigned long *)vmxpde1)[0] = (unsigned long)(kused_mem - KBASE + 0x107);
	//((unsigned long *)vmxpde1)[0] = (unsigned long)(kused_mem - KBASE + 0x1B3);
	print("vmxpde1:%lx\n", ((unsigned long *)vmxpde1)[0]);
	print("vmxpde1:%lx\n", kused_mem - KBASE);
	//kused_mem += 512 * PAGESIZE;
*/
	extern unsigned char vmxcode[];
	memcpy(vmxcode, code, sizeof(code));
	//print("code len :%x\n", sizeof(code));
#if 1
	extern unsigned char trampoline_data[];
	extern unsigned long spa;
	spa = (unsigned long) TRAMPOLINE_BASE;
	memcpy((void *)KPA2VA(spa), trampoline_data, 4096);
	/* copy the AP trampoline code  code */
//	kused_mem += PAGESIZE ;
#endif
	kused_mem = ((kused_mem + PAGESIZE - 1) / PAGESIZE) * PAGESIZE; 
	pages = (struct page *)(kused_mem);	
	kused_mem += sizeof(struct page) * npages;

	//print("mem_size_kb:%lx\n", kused_mem);
	kused_mem = ((kused_mem + PAGESIZE - 1) / PAGESIZE) * PAGESIZE; 

	procs = (struct proc *)(kused_mem);
	memset(procs, 0 , sizeof(struct proc) * 1024);
	kused_mem += sizeof(struct proc) * 1024;
	init_page();

/*	 asm volatile ("movq %0, %%cr3"
                        :
                        : "r" (kpgdir - KBASE)  );
*/

//	check_phead();		
}




