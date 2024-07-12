#include <kern/cache.h>
#include <kern/lib.h>
#include <kern/mm.h>
enum {
	DIRTY,
	UPTODATE
};
struct pagecache {
	int blockno;
	int refcount;
	int status;
	struct page *pp;
	struct pagecache *next;
	struct pagecache *prev;
};
#define MAXPAGECACHE 32
struct pagecache  pcaches[MAXPAGECACHE];
struct pagecache pcachehead;

void free_cache_page(blockno)
{
	struct pagecache *pc = NULL;
	for(pc = pcachehead.next; pc != &pcachehead; pc = pc->next) {
                if(pc->blockno == blockno) {
                        pc->refcount--;
		}
        }
	
}

struct page *read_cache_page(int blockno)
{	
	struct pagecache *pc = NULL;	
	struct page *pp = NULL;

	/* select cache page  in lru list */
	for(pc = pcachehead.prev; pc != &pcachehead; pc = pc->prev) {
		if(pc->refcount == 0) {
//			pp = alloc_cache_page()
			pc->blockno = blockno;
//			pc->pp = pp;
			pc->refcount++;
			return pc->pp;
		}
	}

	/* no free page cache */
	print("no free page cache");
	return NULL;
}
struct page *find_cache_page(int blockno)
{
	struct pagecache *pc = NULL;	
	struct page *pp = NULL;

	/* Don't find blockno cache page  */
	for(pc = &pcachehead; pc != &pcachehead; pc = pc->next) {
		if(pc->blockno == blockno)
			return pc->pp;		
	}

	return NULL;
}


void init_cache_page()
{
	int i;
	struct pagecache *pc = NULL;

	pcachehead.next = &pcachehead;
	pcachehead.prev = &pcachehead;

	for(pc = &pcaches[0]; pc < &pcaches[MAXPAGECACHE]; pc++) {
		pc->next = pcachehead.next;
		pc->prev = &pcachehead;

		pc->refcount = 0;
		pc->blockno = -1;
		pc->status = UPTODATE;
		pc->pp = alloc_page(0);;
	
		pcachehead.next->prev = pc;
		pcachehead.next =  pc;		
		
	}	
}
