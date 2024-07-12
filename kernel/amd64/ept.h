#ifndef __EPT_H
#define __EPT_H
#define VMX_EPT_WB   0x6
#define VMX_EPT_PWL  0x18
#define VMX_EPT_AD  (1ull << 6)
#define VMX_EPT_PTE_X   0x4
#define VMX_EPT_PTE_W   0x2
#define VMX_EPT_PTE_P   0x1
#define VMX_EPT_PTE_FULL (VMX_EPT_PTE_X | VMX_EPT_PTE_W| VMX_EPT_PTE_P)
int mmap_mem(unsigned long *pgdir, void *addr, unsigned long length, void *gpa,  unsigned int prot); 
#endif
