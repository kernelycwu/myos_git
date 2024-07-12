#ifndef _MM_H
#define _MM_H
struct page {
	struct page *link;
	int count;
};
void init_mem();
void  init_page(void);
struct page * alloc_page(int flags);
void free_page(struct page *p);

#define PGZERO  0X1

#define PTE_U   0x4
#define PTE_W   0x2
#define PTE_P   0x1

extern struct page *pages;
/*
#define PGD_SHIFT 22
#define PTE_SHIFT 12

#define KPA2VA(pa)  ((pa) + 0xC0000000) 
#define KVA2PA(va)  ((va) - 0xC0000000)

#define PAGE2PA(pageaddr)  (((pageaddr) - pages) << PTE_SHIFT)
#define PAGE2VA(pageaddr)  (0xc0000000 + PAGE2PA(pageaddr))
#define PA2PAGE(pa) (pages + (pa))
#define VA2PAGE(va) (PA2PAGE(((va)-0xc0000000) >>PGD_SHIFT))
 */
#endif
