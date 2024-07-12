#ifndef MEM_LAYOUT_H
#define MEM_LAYOUT_H
#define PAGESIZE	(4096)
#define UTEXT  (0x8048000)
#define KBASE  (0xC000000)
#define USTACKTOP ((KBASE) - (PAGESIZE))

#define PGD_SHIFT 22
#define PTE_SHIFT 12

#define KPA2VA(pa)  ((pa) + 0xC0000000) 
#define KVA2PA(va)  ((va) - 0xC0000000)

#define PAGE2PA(pageaddr)  (((pageaddr) - pages) << PTE_SHIFT)
#define PAGE2VA(pageaddr)  (0xc0000000 + PAGE2PA(pageaddr))
#define PA2PAGE(pa) (pages + (pa))
#define VA2PAGE(va) (PA2PAGE(((va)-0xc0000000) >>PGD_SHIFT))

#endif
