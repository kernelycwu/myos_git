#include <kern/arch.h>
#include <kern/ide.h>
#include <kern/fs.h>
#include <kern/console.h>
#include <kern/mm.h>
#include <kern/cache.h>

#define BLKSIZE (4096)
#define IMGSIZE (8*4096*4906)
#define INOSIZE (10*4096)
#define NDEBUG 1
#define ROOT_INODE 0
#define BITLENGTH  (sizeof(unsigned int) * 8)
#define NULL 0
struct super_t  *sb = NULL;
unsigned int *blk_bitmap = NULL;
unsigned int *ino_bitmap = NULL;
struct inode_t * ino_table = NULL;

static unsigned int
is_blkempty(unsigned int *bitmap, unsigned int no);
static void set_bitmap(unsigned int *bitmap, unsigned int no);

static int
get_free_no(unsigned int  *bitmap)
{
        int blkno = 0;
        for(blkno = 0; blkno < BLKSIZE * 8 ; blkno++)
                if(is_blkempty(bitmap, blkno))
                        return blkno;
        return  -1;
}

static unsigned int
is_blkempty(unsigned int *bitmap, unsigned int no)
{
        unsigned int res = 0;
        res = bitmap[no / BITLENGTH] & (1 << (no % BITLENGTH));
        return res;
}

static void
set_bitmap(unsigned int *bitmap, unsigned int no) {

        bitmap[no /BITLENGTH] &= ~(1 << (no % BITLENGTH));
}

int get_blk()
{
	int bno;
	bno = get_free_no(blk_bitmap);
	if(bno < 0) {
		print("No free inode number\n");
		return -1;
	}
	set_bitmap(blk_bitmap, bno);
	return bno;	
}

void* bread(int block)
{
	void *buf;
	struct page *pp;
        pp = alloc_page(0); 
        buf = (void *)PAGE2VA(pp);
        readblk(buf, 8*block, 8);
	return buf;
/*
	struct page *pp;
	void *buf;

	pp = find_cache_page(block);
	if(pp != NULL)
		return (void *)PAGE2VA(pp);

	pp = read_cache_page(block);
	if(pp != NULL) {
		buf = (void *)PAGE2VA(pp);
		readblk(buf, 8*block, 8);
		return buf;	
	}
	panic("no empty cache page");
	return NULL;
*/
}

void read_superblock()
{
	struct page *pp;
	void *buf;
	
	pp = alloc_page(0); 
	buf = (void *)PAGE2VA(pp);
	readblk(buf, 8*1, 8);
	sb = (struct super_t *)buf;
	print("\n===%x,%x\n", sb->magic[0],sb->magic[1]);

	pp = alloc_page(0);
        readblk((void *)PAGE2VA(pp), 8*2, 8);
	blk_bitmap = (unsigned int*)PAGE2VA(pp);
			
	
	pp = alloc_page(0);
        readblk((void *)PAGE2VA(pp), 8*3, 8);
        ino_bitmap = (unsigned int *)PAGE2VA(pp);

	pp = alloc_page(0);
        readblk((void *)PAGE2VA(pp), 8*4, 8);
        ino_table = (struct inode_t *)PAGE2VA(pp);
}
int
walk_block(struct inode_t *p, unsigned int fileno, unsigned short **diskno, int alloc)
{

	int bno;
	int level, level1;
	unsigned short *blk = NULL;

	if(fileno < 10) {
		*diskno = &p->block[fileno];
	} else if (fileno < 1024 * 2 + 10) {
		level = (fileno - 10)/2048 + 10;
		if (p->block[level] == 0) {
			if(alloc == 1) {
				bno = get_free_no(blk_bitmap);
				if(bno < 0) {
					print("No free inode number\n");
					return -1;
				}
				p->block[level] = bno;
				set_bitmap(blk_bitmap, bno);				
				
			} else {
				*diskno = 0;
				return -1;
			}
		}
		blk = 	(unsigned short *)bread(p->block[level]);
		//blk = (unsigned short *)(img_addr + p->block[level] * BLKSIZE);

		level1 = (fileno - 10) % 2048;
		*diskno = &blk[level1];	
	}
	return 0;	
	
}
int walk_path(char *fname, struct inode_t** inode) 
{
	struct inode_t *p = &ino_table[ROOT_INODE];
	//assert(p->di_size % BLKSIZE  != 0);
	int i = 0, j = 0;
	int r;
	unsigned short *diskno;
	struct direct_t *pdir;

	unsigned int fileno = p->di_size / BLKSIZE;

	for(i = 0; i < fileno; i++) {

		r = walk_block(p, i, &diskno, 0);
		if(r < 0) {
			print("walk block error\n");
			return -2;
		}
		if(*diskno != 0) {
			print("diskno=%d", *diskno);
			//pdir = (struct direct_t *)(img_addr + *diskno * BLKSIZE);
			pdir = (struct direct_t *)bread(*diskno);
			while(j < BLKSIZE / sizeof(struct direct_t)) {
				if(strcmp(pdir->name, fname) == 0) {
//					print("aaaaaaaaaaaaa:%s, %s",pdir->name, fname);
//					panic("bbbbbbbbbbb\n");

					*inode = &ino_table[pdir->ino];						
					print("please change filename\n");
					return 0;
				}
				pdir++;
				j++;
			} 
		}
	}
	return -1;
}
void init_fs()
{
	print("init my file system\n");
	init_cache_page();
	read_superblock();	
	//walk_path("xxx", NULL);	
}
