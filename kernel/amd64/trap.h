#ifndef _TRAP_H
#define _TRAP_H
/* 16 bytes */
struct gdt_desc {
        unsigned short limit0  ;
        unsigned short base0   ;
        unsigned char base1    ;
        unsigned char type     ;
        unsigned char limit1   ;
        unsigned char base2    ;
	unsigned int base3;
	unsigned int reserved_zero;
};

/* 16 bytes */
struct idt_desc {
        unsigned short offset0  ;
        unsigned short selector ;
        unsigned char reserve;
        unsigned char type;
        unsigned short offset1 ;
        unsigned int offset2 ;
	unsigned int  reserved_ign;
};

struct idt_reg {
        unsigned short limit ;
        unsigned long base;
}__attribute__((packed));

struct tss_struct {
	unsigned int reserved_ign;
	unsigned long rsp0;
	unsigned long rsp1;
	unsigned long rsp2;
	unsigned long reserved_ign1;
	unsigned long ist1;
	unsigned long ist2;
	unsigned long ist3;
	unsigned long ist4;
	unsigned long ist5;
	unsigned long ist6;
	unsigned long ist7;
	unsigned long reserved_ign2;
	unsigned short reserved_ign3;
	unsigned short reserved_iomap;	
        //unsigned long           io_bitmap[65536/(8*sizeof(unsigned int)) + 1];
}__attribute__((packed));

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
