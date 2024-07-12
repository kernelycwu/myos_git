.code32
.text
.globl _start
_start:
    #movw $0x1234,0x472			# warm boot

# Reset the stack pointer in case we didn't come from the loader
    movl $0x7c00,%esp

/* disable paging */
movl %cr0, %eax
andl $0xfffffffe, %eax
movl %eax, %cr0

/* Set the PAE enable bit in CR4 */
movl %cr4, %eax
orl $0x00000020, %eax
movl %eax, %cr4

#call verify_vmx
# build an early boot pml4 at physical address pml4phys 

    #initializing the page tables
//    movl $pml4,%edi
//    xorl %eax,%eax
//    movl $((4096/4)*5),%ecx  # moving these many words to the 6 pages with 4 second level pages + 1 3rd level + 1 4th level pages 
//    rep stosl
/*
     movl $(pdpt1 + 0x007), (pml4  + 0 * 8) //pml4
     movl $(pde1 + 0x007), (pdpt1  + 0 * 8) //pdpt0
     movl $(pdpt2 + 0x007), (pml4 + 511 * 8) //pml4
     movl $(pde2 + 0x007), (pdpt2 + 510 * 8) //pdpt1

     movl $0x183, %eax
     movl $0, %ecx
     movl $pde1,%edi
     movl $pde2,%ebx
1:   movl %eax, (%edi)
     movl %eax, (%ebx)
     addl $0x8,%edi
     addl $0x8,%ebx
     addl  $0x00200000,%eax
     addl $1, %ecx
     cmp  $0x63, %ecx
     jne 1b  
*/
/* 0xffff ffff 8020 0000 */

movl $(pdpt1 + 0x007), (pml4  + 0 * 8) //pml4
movl $(pde1 + 0x007), (pdpt1  + 0 * 8) //pdpt0
movl $(0 + 0x183), (pde1 + 0 * 8 )  //pde1
movl $(0x200000 + 0x183), (pde1 + 1 * 8 )  //pde1
movl $(0x400000 + 0x183), (pde1 + 2 * 8 )  //pde1
movl $(0x600000 + 0x183), (pde1 + 3 * 8 )  //pde1
movl $(0x800000 + 0x183), (pde1 + 4 * 8 )  //pde1


movl $(pdpt2 + 0x007), (pml4 + 511 * 8) //pml4
movl $(pde2 + 0x007), (pdpt2 + 510 * 8) //pdpt1
movl $(0 + 0x183), (pde2 + 0 * 8 )  //pde1
movl $(0x200000 + 0x183), (pde2 + 1 * 8 )  //pde1
movl $(0x400000 + 0x183), (pde2 + 2 * 8 )  //pde1
movl $(0x600000 + 0x183), (pde2 + 3 * 8 )  //pde1
movl $(0x800000 + 0x183), (pde2 + 4 * 8 )  //pde1

    # set the cr3 register
    movl $pml4,%eax
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

    #jump to long mode with CS=0 and

    movl $boot_gdtdesc,%eax
    lgdt (%eax)
    pushl $0x8
    movl $entry64,%eax
    pushl %eax

    .globl jumpto_longmode
    .type jumpto_longmode,@function
jumpto_longmode:
    lret

verify_vmx:
     movl $0x1, %eax
     cpuid
     test $(1 << 5), %ecx
     jz  no_vmx
     ret
no_vmx: 
     jmp no_vmx


.data
    .p2align 2
boot_gdt:
.global boot_gdt
    .quad  0x0000000000000000
    .quad  0x00af9a000000ffff            #64 bit CS
    .quad  0x00cf92000000ffff            #64 bit DS
   // .quad  0x00affa000000ffff            #64 bit User CS
   // .quad  0x00cff2000000ffff            #64 bit User DS
   // .quad  0x0000000000000000            #64 bit TSS
   // .quad  0x0000000000000000            #64 bit TSS
.global boot_gdtdesc
boot_gdtdesc:
    .word 0x17 
    .quad boot_gdt

    .p2align 12
.globl boot_pgkdir
boot_pgkdir:
pml4:
    .fill 512, 8, 0

pdpt1:
    .fill 512, 8, 0

pdpt2:
    .fill 512, 8, 0

pde1:
    .fill 512, 8, 0

pde2:
    .fill 512, 8, 0

//.globl boot_pgkdir
//boot_pgkdir:
//    .space 5*4096

