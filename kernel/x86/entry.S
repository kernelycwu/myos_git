.text
.align  4
#.global _start
.type _start, @function
_start:
	/*initial page table */
	xorl %eax, %eax
	movl $(kpgpte - 0xc0000000), %edi
	movl $0, %esi
	movl $1024, %ecx

1:
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)
	
	addl $4096, %esi
	addl $4, %edi
	decl %ecx
	testl %ecx, %ecx
	jne	1b

 	/* initial page dir */
	//movl $(0x000B8000 | 0x003), kpgpte - 0xC0000000 + 1023 * 4
	movl $(kpgpte - 0xC0000000 + 0x003), kpgdir - 0xC0000000 + 0
	movl $(kpgpte - 0xC0000000 + 0x003), kpgdir - 0xC0000000 + 768 * 4
	
		
	/*eable paging */
	movl $(kpgdir-0xc0000000), %eax
	movl %eax, %cr3
	movl $(stacktop-0xc0000000), %esp

	lgdtl gdtdesc
	
	movl %cr0, %eax
	orl  $0x80000001, %eax
	movl %eax, %cr0
	jmp 1f
1:
	leal 1f, %eax
	jmp *%eax
1:
	movl %cr3, %ecx
	movl %ecx, %cr3
	
	movl $stacktop, %esp
	call kernmain
loop:  
	jmp loop
 
.section .data, "aw"
.data
.global kpgdir
kpgdir:
	.fill 1024, 4, 0
kpgpte:
	.fill 1024, 4, 0
kernstack:
	 .fill 1024, 4, 0
.global stacktop
stacktop:
 .p2align        4       /* force 4-byte alignment */
.global idt
idt:
	.fill 256, 8, 0
.global idtdesc	
idtdesc:
	.word 2047
	.long idt
	
 .p2align        4       /* force 4-byte alignment */
.global gdt
gdt:
        .word   0, 0
        .byte   0, 0, 0, 0

        /* code segment */
        .word   0xFFFF, 0
        .byte   0, 0x9A, 0xCF, 0

        /* data segment */
        .word   0xFFFF, 0
        .byte   0, 0x92, 0xCF, 0

	/* user code segment */
	.word   0xFFFF, 0
	.byte   0, 0xFA, 0xCF, 0

	/* user data segment */
	.word   0xFFFF, 0
	.byte   0, 0xF2, 0xCF, 0

	/* tss segment */
	.word   0, 0
        .byte   0, 0, 0, 0

/* this is the GDT descriptor */
.global gdtdesc
gdtdesc:
        .word   0x2F                    /* limit */
        .long   gdt                     /* addr */


