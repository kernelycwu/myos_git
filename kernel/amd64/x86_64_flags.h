#define X86_CR0_PG      0x80000000 /* Paging */
#define X86_CR0_PE      0x00000001 /* Protection Enable */
#define X86_CR4_PAE     0x00000020 /* enable physical address extensions */
#define EFER_MSR		0xc0000080 /* extended feature register */
#define _EFER_LME		8  /* Long mode enable */
#define __KERNEL_CS	(2 * 8)
#define __KERNEL_DS	(3 * 8)
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable

