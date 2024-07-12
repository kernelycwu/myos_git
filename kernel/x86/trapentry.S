/* No Erorr Code */
.align 2;
divide_trap:
.global divide_trap 
	pushl $0
	pushl $0 ; 
        jmp do_alltrap;
.align 2;
debug_trap:
.global debug_trap
	pushl $0 ; 
	pushl $1
        jmp do_alltrap;
breakpoint_trap:
.global breakpoint_trap
	pushl $0 ; 
	pushl $3
        jmp do_alltrap;

overflow_trap:
.global overflow_trap
	pushl $0 ; 
	pushl $4
        jmp do_alltrap;

bound_trap:
.global bound_trap
	pushl $0 ; 
	pushl $5
        jmp do_alltrap;

invalid_trap:
.global invalid_trap
	pushl $0 ; 
	pushl $6
        jmp do_alltrap;

coprocessor_not_avail_trap:
.global coprocessor_not_avail_trap
	pushl $0 ; 
	pushl $7
        jmp do_alltrap;

segment_trap:
.global segment_trap
	pushl $0 ; 
	pushl $9
        jmp do_alltrap;

coprocessor_trap:
.global coprocessor_trap
	pushl $0 ; 
	pushl $16
	jmp do_alltrap;
.align 2;
syscall_trap:
.global syscall_trap
        pushl $0 ; 
        pushl $20
        jmp do_alltrap;

/* Error Code */
double_trap:
.global double_trap
	pushl $8
	jmp do_alltrap;

invalid_tss_trap:
.global invalid_tss_trap
	pushl $10
	jmp do_alltrap;

segmentnotpresent_trap:
.global segmentnotpresent_trap
	pushl $11
	jmp do_alltrap;
stack_trap:
.global stack_trap
	pushl $12
	jmp do_alltrap;

protection_trap:
.global protection_trap
	pushl $13
	jmp do_alltrap;

page_fault_trap:
.global page_fault_trap
	pushl $14
	jmp do_alltrap;
.align 2;
timer_intr:
.global timer_intr
	pushl $0
	pushl $32
	jmp do_alltrap;
kdb_intr:
.global kdb_intr
	pushl $0
	pushl $33
	jmp do_alltrap;

do_alltrap:
	pushl %es
	pushl %ds
	pushal
	movl $0x10, %eax
	movl %eax, %es
	movl %eax, %ds

	pushl %esp
	call do_trap
	popl %esp

	popal
	popl %ds
	popl %es
	addl $8, %esp
	iret
