#include <kern/lib.h>
#include  "trap.h"
//#include <kern/syscall.h>
#include <kern/proc.h>
#include <kern/mm.h>
//#include <kern/mm_inl.h>
extern unsigned char stacktop[];
extern void *gdtdesc, *idtdesc;
extern struct proc *current;
struct tss_struct tss = { 0, 0, (unsigned int)&stacktop[0], 0x10, 0,
	0, 0 , 0 , 0, 0 ,0 ,0 ,0,
	0, 0 , 0 , 0, 0 ,0 ,0 ,0,
	0, 0 , 0 , 0, 0 ,0 ,0 ,0,
	0, 0 , 0 , 0, 0 ,0 ,0 ,0,
	0, 
	//{0, } 
	
};
	
static void 
set_idt_desc(unsigned char idtno, void *addr,  unsigned char type)
{
	extern unsigned char  idt[];
	struct idt_desc *idt_table = (struct idt_desc *)idt;
	
	idt_table[idtno].offset0 = (unsigned int)addr & 0xffff;
	idt_table[idtno].selector = 0x08;
	idt_table[idtno].reserve = 0;
	idt_table[idtno].type = type;
	idt_table[idtno].offset1 =((unsigned int)addr >> 16) & 0xffff;
}

static void 
set_tss_desc(void *addr,  unsigned int limit,  unsigned char type)
{
	extern unsigned char  gdt[];
	struct gdt_desc *tss_desc = (struct gdt_desc *)(gdt + 0x28);

	tss_desc->limit0 = limit & 0xffff;
	tss_desc->base0 = ((unsigned int )addr) & 0xffff;
	tss_desc->base1 = ((unsigned int )addr >> 16) & 0xff;
	tss_desc->type = type ;
	tss_desc->limit1 = 0;
	tss_desc->base2 = ((unsigned int )addr >> 24) & 0xff;	
}
void init_cpu()
{
	asm volatile ("lgdt %0" : "=m"(gdtdesc));
	asm volatile ("lidt %0" : "=m"(idtdesc));
	set_tss_desc(&tss, sizeof(struct tss_struct), 0x89 );
	asm volatile ( "ltr %%ax" : : "a" (0x28) );	
}
void init_trap()
{
/*
	asm volatile ("lgdt %0" : "=m"(gdtdesc));
	set_tss_desc(&tss, sizeof(struct tss_struct), 0x89 );
	asm volatile ( "ltr %%ax" : : "a" (0x28) );
*/
	set_idt_desc(0, divide_trap, 0x8F);
	set_idt_desc(1, debug_trap, 0x8F);
	set_idt_desc(3, breakpoint_trap, 0xEF);
	set_idt_desc(4, overflow_trap, 0xEF);
	set_idt_desc(5, bound_trap, 0xEF);
	set_idt_desc(6, invalid_trap, 0xEF);
	set_idt_desc(7, coprocessor_not_avail_trap, 0xEF);
	set_idt_desc(8, double_trap, 0xEF);
	//set_idt_desc(9, segment_trap, 0x8F);
	set_idt_desc(9, segment_trap, 0xEF);


	set_idt_desc(10, invalid_tss_trap, 0xEF);
	set_idt_desc(11, segmentnotpresent_trap, 0xEF);
	set_idt_desc(12, stack_trap, 0xEF);
	set_idt_desc(13, protection_trap, 0xEF);
	//set_idt_desc(13, protection_trap, 0x8F);

	set_idt_desc(14, page_fault_trap, 0xEF);
	set_idt_desc(16, coprocessor_trap, 0xEF);
	/* syscall: DPL 11 */
	//set_idt_desc(20, syscall_trap, 0xEF);
	set_idt_desc(20, syscall_trap, 0xEE);
	
	/*interrupt:  P |DPL|0 1 1 1 0 */
	/*Fixme:TODO */
	//int i =0;
	//for(i = 32; i < 48; i++)
	init_cpu();	
	set_idt_desc(32, timer_intr, 0x8E);
	set_idt_desc(32 + 1, kdb_intr, 0x8E);

//	asm volatile ("lidt %0" : "=m"(idtdesc));
	//asm volatile ("lgdt %0" : "=m"(gdtdesc));
	//set_tss_desc(&tss, sizeof(struct tss_struct), 0x89 );
	//asm volatile ( "ltr %%ax" : : "a" (0x28) );

}

extern int do_syscall(unsigned int num, unsigned int p0, unsigned int p1,
                         unsigned int p2,unsigned int p3,unsigned int p4);

unsigned int  read_cr2(void)
{
	unsigned int err_va;
	asm volatile("movl %%cr2, %0"
		:"=r"(err_va)
		);
	return err_va;	
}

static void print_regs(struct pt_regs *pt)
{
	print("==========pt_reg begin============\n");
	print("ebx     %x\n", pt->ebx);
	print("edx     %x\n", pt->edx);
	print("ecx     %x\n", pt->ecx);
	print("eax     %x\n", pt->eax);
	print("ds      %x\n", pt->ds);
	print("es      %x\n", pt->es);
	print("trapno  %x\n", pt->num);
	print("errcode %x\n", pt->err_code);
	print("eip     %x\n", pt->eip);
	print("cs      %x\n", pt->cs);
	print("eflags  %x\n", pt->eflags);
	print("esp     %x\n", pt->esp);
	print("ss      %x\n", pt->ss);
	print("==========pt_reg end ============\n");
}
/* Page-Fault Error Code Format */
#define PFEC_U 0x4 /* in  user mode */
#define PFEC_W 0x2 /* the fault was a write */
#define PFEC_P 0x1 /* page-level protection violation */
 
extern void kdb_handler();
void do_trap(struct pt_regs *pt)
{
	unsigned int cr2;
	unsigned int trapno = pt->num;
	struct page *pp = 0;
	int r;
	
	static int i=0;
	if(current != 0 ) {
		//if ((pt->cs & 3) == 3)
		memcpy((void *)&current->pregs, (void *)pt, sizeof(struct pt_regs));
	}


	switch (trapno) {
		case 0x20:
//			print("********************************");
                       // asm volatile ("hlt");
			schedule();
			break;
		case 0x21:

			kbd_handler();
			break;

		case 14:
			cr2 = read_cr2();

			//print("Pagefault cr2:%x, errcode:%x\n", cr2, pt->err_code);
			if(!(pt->err_code & PFEC_U)) {
				print_regs(pt);
				panic("page fault in kernel");
			}
			/* Page fault in user mode */
			cr2 = cr2 & 0xfffff000;

			if(!(pt->err_code & PFEC_P)) { // not present
				pp = alloc_page(1);
				/* set all permission */
				r = insert_page(current->pgdir, cr2, pp,  PTE_U|PTE_P|PTE_W);			
			} else
			if(pt->err_code & PFEC_W) { //cow 		
				pp = alloc_page(1);
				memcpy((void *)PAGE2VA(pp), (void *)cr2, 4096);

				remove_page(current->pgdir, cr2);
				r = insert_page(current->pgdir, cr2, pp,  PTE_U|PTE_P|PTE_W);

			} 

			//print_regs(pt);
			//asm volatile ("hlt");		
			break;
		case 20:
			r = do_syscall(pt->eax, pt->ebx, pt->ecx, pt->edx, pt->esi, pt->edi);
			pt->eax = r ;
			//run_proc(current);
			break;
		default:
			print_regs(pt);
			asm volatile ("hlt");		
			break;
	}
	//asm volatile ("hlt");		
}
