.code16
.global trampoline_data
trampoline_data:
r_base = .

	cli                     # We should be safe anyway
#        wbinvd
        mov     %cs, %ax        # Code and data in the same place
        mov     %ax, %ds
        mov     %ax, %es
        mov     %ax, %ss
                                        # Setup stack
 #       movw    $(trampoline_stack_end - r_base), %sp
        mov     %cs, %ax
        movzx   %ax, %esi               # Find the 32bit trampoline location
        shll    $4, %esi

                                        # Fixup the vectors
        addl    %esi, startup_32_vector - r_base
        addl    %esi, startup_64_vector - r_base
        addl    %esi, tgdt + 2 - r_base # Fixup the gdt pointer

 	lidtl   tidt - r_base   # load idt with 0, 0
        lgdtl   tgdt - r_base   # load gdt with whatever is appropriate

	movl %cr0, %eax
	orb  $1, %al
	movl %eax, %cr0

   	ljmpl   *(startup_32_vector - r_base)

.code32
.balign 4
startup_32:
	movl    $0x18, %eax      # Initialize the %ds segment register
        movl    %eax, %ds


	movl	$0x7c00, %esp
	/* Set the PAE enable bit in CR4 */
	movl %cr4, %eax
	orl $0x00000020, %eax
	movl %eax, %cr4

	# set the cr3 register
	movl $boot_pgkdir,%eax
	movl %eax, %cr3

	/* Enable long mode by setting the LME flag (bit 8) in MSR 0xC0000080 (aka EFER) */
	movl $0xc0000080, %ecx
	rdmsr
	orl $0x00000100, %eax
	wrmsr

	/* Enable paging */
	movl %cr0, %eax
	orl $0x80000001, %eax
	movl %eax, %cr0
	ljmp    *(startup_64_vector - r_base)(%esi)

.code64
.balign 4
startup_64:
	movq    $smp_entry64, %rax
        jmp     *%rax
	
tidt:
        .word   0                       # idt limit = 0
        .word   0, 0                    # idt base = 0L

        # Duplicate the global descriptor table
        # so the kernel can live anywhere
        .balign 4
tgdt:
        .short  tgdt_end - tgdt         # gdt limit
        .long   tgdt - r_base
        .short  0
        .quad   0x00cf9b000000ffff      # __KERNEL32_CS
        .quad   0x00af9b000000ffff      # __KERNEL_CS
        .quad   0x00cf93000000ffff      # __KERNEL_DS
tgdt_end:

        .balign 4
startup_32_vector:
        .long   startup_32 - r_base
        .word   0x8, 0

        .balign 4
startup_64_vector:
        .long   startup_64 - r_base
        .word   0x10, 0

.global trampoline_end
trampoline_end:
