#ifndef CACHE_H
#define CACHE_H
/*
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
*/
void init_cache_page(void);
struct page  *find_cache_page(int blockno);
#endif

