#ifndef _TRAP_H
#define _TRAP_H
struct gdt_desc {
        unsigned short limit0  ;
        unsigned short base0   ;
        unsigned char base1    ;
        unsigned char type     ;
        unsigned char limit1   ;
        unsigned char base2    ;
};
struct idt_desc {
        unsigned short offset0  ;
        unsigned short selector ;
        unsigned char reserve;
        unsigned char type;
        unsigned short offset1 ;
};
struct idt_reg {
        unsigned short limit ;
        unsigned int base;
};
struct tss_struct {
	unsigned short link, linkh;
	unsigned int esp0;
	unsigned short ss0, ss0h;
	unsigned int esp1;
	unsigned short ss1, ss1h;
	unsigned int esp2;
        unsigned short ss2, ss2h;
	unsigned int cr3;
	unsigned int eip;
	unsigned int eflags;
	unsigned int eax;
	unsigned int ecx;
	unsigned int edx;
	unsigned int ebx;
	unsigned int esp;
	unsigned int ebp;
	unsigned int esi;
	unsigned int edi;
	unsigned short es, esh;
	unsigned short cs, csh;
	unsigned short ss, ssh;
	unsigned short ds, dsh;
	unsigned short fs, fsh;
	unsigned short gs, gsh;
	unsigned short ldt, ldth;
	unsigned short   trap;
	unsigned short   iomap;
        //unsigned long           io_bitmap[65536/(8*sizeof(unsigned int)) + 1];
};

extern void divide_trap();
extern void debug_trap();
extern void breakpoint_trap();
extern void overflow_trap();
extern void bound_trap();
extern void invalid_trap();
extern void coprocessor_not_avail_trap();
extern void segment_trap();
extern void coprocessor_trap();
extern void syscall_trap();
/* Error Code */
extern void double_trap();
extern void invalid_tss_trap();
extern void segmentnotpresent_trap();
extern void stack_trap();
extern void protection_trap();
extern void page_fault_trap();
/*interrupt */
extern void timer_intr();
extern void kdb_intr();
#endif
