/* No Erorr Code */
.align 2;
divide_trap:
.global divide_trap 
	pushq $0
	pushq $0 ; 
        jmp do_alltrap;
.align 2;
debug_trap:
.global debug_trap
	pushq $0 ; 
	pushq $1
        jmp do_alltrap;
breakpoint_trap:
.global breakpoint_trap
	pushq $0 ; 
	pushq $3
        jmp do_alltrap;

overflow_trap:
.global overflow_trap
	pushq $0 ; 
	pushq $4
        jmp do_alltrap;

bound_trap:
.global bound_trap
	pushq $0 ; 
	pushq $5
        jmp do_alltrap;

invalid_trap:
.global invalid_trap
	pushq $0 ; 
	pushq $6
        jmp do_alltrap;

coprocessor_not_avail_trap:
.global coprocessor_not_avail_trap
	pushq $0 ; 
	pushq $7
        jmp do_alltrap;

segment_trap:
.global segment_trap
	pushq $0 ; 
	pushq $9
        jmp do_alltrap;

coprocessor_trap:
.global coprocessor_trap
	pushq $0 ; 
	pushq $16
	jmp do_alltrap;
.align 2;
syscall_trap:
.global syscall_trap
        pushq $0 ; 
        pushq $20
        jmp do_alltrap;

/* Error Code */
double_trap:
.global double_trap
	pushq $8
	jmp do_alltrap;

invalid_tss_trap:
.global invalid_tss_trap
	pushq $10
	jmp do_alltrap;

segmentnotpresent_trap:
.global segmentnotpresent_trap
	pushq $11
	jmp do_alltrap;
stack_trap:
.global stack_trap
	pushq $12
	jmp do_alltrap;

protection_trap:
.global protection_trap
	pushq $13
	jmp do_alltrap;

page_fault_trap:
.global page_fault_trap
	pushq $14
	jmp do_alltrap;
.align 2;
timer_intr:
.global timer_intr
	pushq $0
	pushq $32
	jmp do_alltrap;
kdb_intr:
.global kdb_intr
	pushq $0
	pushq $33
	jmp do_alltrap;

do_alltrap:
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

	movq  %rsp, %rdi
	call do_trap
	
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbp
	popq %rbx
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rax
	popq %rcx
	popq %rdx
	popq %rsi
	popq %rdi

	/*skip num & error code */
	addq $16, %rsp
	iretq
