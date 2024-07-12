#ifndef MEM_LAYOUT_H
#define MEM_LAYOUT_H
#define PAGESIZE	(4096)
//#define UTEXT  (0x00800000)
#define UTEXT  (0x0000000080000000)
#define KBASE  (0xFFFFFFFF80000000)
//#define USTACKTOP   ((1UL << 47) - PAGESIZE)
#define USTACKTOP   (0x0000000090000000 - PAGESIZE)
//#define USTACKTOP ((KBASE) - (PAGESIZE))

#define PTE_SHIFT (12)
#define PGD_SHIFT (PTE_SHIFT + 9)
#define PDP_SHIFT (PGD_SHIFT + 9)
#define PML_SHIFT (PDP_SHIFT + 9)

#define KVA2PA(va)  ((va) - KBASE)
#define KPA2VA(pa)  ((pa) + KBASE)
 
#define PAGE2PA(pageaddr)  (((pageaddr) - pages) << PTE_SHIFT)
#define PAGE2VA(pageaddr)  (KBASE + PAGE2PA(pageaddr))

#define PA2PAGE(pa) (pages + (pa))
//#define VA2PAGE(va) (PA2PAGE(((va)-KBASE) >>PGD_SHIFT))
#define VA2PAGE(va) (PA2PAGE(((va)-KBASE) >>PTE_SHIFT))
#endif
