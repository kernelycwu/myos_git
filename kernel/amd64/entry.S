.code64
.global entry64
entry64:
       /* Setup data segments. */
        xorl    %eax, %eax
        movl    %eax, %ds
        movl    %eax, %es
        movl    %eax, %ss
        movl    %eax, %fs
        movl    %eax, %gs

        lldt    %ax
        movq    $0x0,%rbp                       # nuke frame pointer

	#Set CR4's bit 13 to 1. This bit enables the VMX operations.
	movq    %cr4, %rax
	orq	$0x2000, %rax
	movq   %rax, %cr4

	#Set CR0's bit 5 to 1 (NE) - this is required for the VMXON to succeed
	movq	%cr0, %rax
	orq	$0x20, %rax
	movq	%rax, %cr0
	#

	movq $vmxpml4,%rdi
	xorq %rax,%rax
	movq $((4096/8)*5), %rcx  # moving these many words to the 6 pages with 4 second level pages + 1 3rd level + 1 4th level pages 
	rep stosq


	movq $(vmxpdpt1 - 0xFFFFFFFF80000000 + 0x007), (vmxpml4  + 0 * 8) //pml4
	movq $(vmxpde1 - 0xFFFFFFFF80000000 + 0x007), (vmxpdpt1  + 0 * 8) //pdpt1
	movq $(vmxpte1 - 0xFFFFFFFF80000000 + 0x007), (vmxpde1  + 0 * 8) //pdet1
	movq $(vmxcode - 0xFFFFFFFF80000000 + 0x007), (vmxpte1  + 0 * 8) //pdet1
//      movq $(0 + 0x183), (vmxpde1 + 0 * 8 )  //pde1
//	movq $(0x200000 + 0x183), (vmxpde1 + 1 * 8 )  //pde1
//	movq $(0x400000 + 0x183), (vmxpde1 + 2 * 8 )  //pde1
//	movq $(0x600000 + 0x183), (vmxpde1 + 3 * 8 )  //pde1

        # Set the stack pointer
        movabs  $(stacktop),%rax
        movq  %rax,%rsp

        # now to C code
	movabs $kernmain, %rax
        call *%rax

        # Should never get here, but in case we do, just spin.
spin:   jmp     spin

.global smp_entry64
smp_entry64:
	   /* Setup data segments. */
        xorl    %eax, %eax
        movl    %eax, %ds
        movl    %eax, %es
        movl    %eax, %ss
        movl    %eax, %fs
        movl    %eax, %gs

        lldt    %ax
        movq    $0x0,%rbp                       # nuke frame pointer

	//ap id 
        movl    $1, %eax
        cpuid
        shrl    $24, %ebx

        movq    %rbx, %rdi
        //set stackstop
	movabs  $(stacktop),%rax
        movq  %rax,%rsp

        shll    $12,  %ebx
        subq    %rbx, %rsp
	//
1:	pause
	cmpb $0, bspdone
	jz  1b

	lock incb aprunning
        call ap_startup

.data
###################################################################
# boot stack
###################################################################
    .p2align        4       /* force 16-byte alignment */
.global idt
idt:
        .fill 256*2, 8 , 0
.global idtdesc
idtdesc:
        .word 4095
        .quad idt

    .p2align 2

.global gdt
gdt:
    .quad  0x0000000000000000
    .quad  0x00af9a000000ffff            #64 bit CS
    .quad  0x00cf92000000ffff            #64 bit DS
    .quad  0x00affa000000ffff            #64 bit User CS
    .quad  0x00cff2000000ffff            #64 bit User DS
    .quad  0x0000000000000000            #64 bit TSS
    .quad  0x0000000000000000            #64 bit TSS
.global gdtdesc
gdtdesc:
    .word 0x37
    .quad gdt

    .p2align        12         # force page alignment
        .globl          bootstack
bootstack:
        .space          4096
	.space          4096
	.space          4096
        .globl          stacktop
stacktop:


.p2align 12
# ept 
.globl vmx_pgkdir
vmx_pgkdir:
vmxpml4:
.fill 512, 8, 0

vmxpdpt1:
.fill 512, 8, 0

.global vmxpde1
vmxpde1:
.fill 512, 8, 0

vmxpte1:
.fill 512, 8, 0

.global vmxcode
vmxcode:
.fill 512, 8, 0

