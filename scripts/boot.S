.text

.global _start
_start:
.code16
	cli
	xorw %ax, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss

	lgdtl gdtdesc

	movl %cr0, %eax
	orb  $1, %al
	movl %eax, %cr0
	
	ljmpl $0x8, $protect_mode

.code32
protect_mode:
	movl   $0x10, %eax
	movl  %eax, %ds
	movl  %eax, %ss
	movl  %eax, %fs
	movl  %eax, %es

#	movl $(stack + 0x4000), %esp
#	call main
	#hlt
	jmp 0x100000
hang:
	jmp hang



.p2align        2       /* force 4-byte alignment */
gdt:
        .word   0, 0
        .byte   0, 0, 0, 0

        /* code segment */
        .word   0xFFFF, 0
        .byte   0, 0x9A, 0xCF, 0

        /* data segment */
        .word   0xFFFF, 0
        .byte   0, 0x92, 0xCF, 0

/* this is the GDT descriptor */
gdtdesc:
        .word   0x17                    /* limit */
        .long   gdt                     /* addr */



	
