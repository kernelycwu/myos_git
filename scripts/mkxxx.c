#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include "mkxxx.h"

//#define DEBUG	1

#define BLKSIZE (4096)
#define IMGSIZE (8*4096*4906)
#define INOSIZE (10*4096)
#define NDEBUG 1
#define ROOT_INODE 0
#define BITLENGTH  (sizeof(unsigned int) * 8)
unsigned int *blk_bitmap = NULL;
unsigned int *ino_bitmap = NULL;
struct inode_t * ino_table = NULL;

void *img_addr;
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
	return 	-1;
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
					printf("No free inode number\n");
					return -1;
				}
				p->block[level] = bno;
				set_bitmap(blk_bitmap, bno);				
				
			} else {
				*diskno = 0;
				return -1;
			}
		}
		blk = (unsigned short *)(img_addr + p->block[level] * BLKSIZE);

		level1 = (fileno - 10) % 2048;
		*diskno = &blk[level1];	
	}
	return 0;	
	
}

static int walk_path(char *fname) 
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
			printf("walk block error\n");
			return -2;
		}
		if(*diskno != 0) {
			pdir = (struct direct_t *)(img_addr + *diskno * BLKSIZE);
			while(j < BLKSIZE / sizeof(struct direct_t)) {
				if(strcmp(pdir->name, fname) == 0) {
					printf("please change filename\n");
					return 0;
				}
				pdir++;
				j++;
			} 
		}
	}
	return -1;
}


static int 
alloc_file(struct direct_t **dir)
{
	struct inode_t *p = &ino_table[ROOT_INODE];
	assert(p->di_size % BLKSIZE  == 0);
	int i = 0, j = 0;
	int r;
	unsigned short *diskno;
	struct direct_t *pdir;
	int bno;	

	unsigned int fileno = p->di_size / BLKSIZE;
	for(i = 0; i < fileno; i++) {
		r = walk_block(p, i, &diskno, 1);
		if(r < 0) {
			printf("walk block error\n");
			return -2;
		}
		if(*diskno != 0) {
			pdir = (struct direct_t *)(img_addr + *diskno * BLKSIZE);
			while(j < BLKSIZE / sizeof(struct direct_t)) {
				if(*pdir->name == '\0' ) {
					*dir = pdir;
					return 0;
				}
				pdir++;
				j++;
			}
		}
	}

	r = walk_block(p, i, &diskno, 1);
	if(r < 0) {
		printf("walk block error\n");
		return -2;
	}
		
	bno = get_free_no(blk_bitmap);
	if(bno < 0) {
		printf("No free inode number\n");
		return -1;
	}
	p->di_size += BLKSIZE;
	*diskno = bno;
#ifdef DEBUG
	printf("==============:%d",bno);
#endif
	set_bitmap(blk_bitmap, bno);
	*dir = (struct direct_t *)(img_addr + bno * BLKSIZE);	
	return 0;
}

#define MIN(x, y) (x)<(y) ? (x):(y)
static int 
write_file(char *fname, struct inode_t *pino) {

	int fd, r;
	unsigned short *diskno;
	int bno, count = 0;
	int i = 0, bn = 0;
	unsigned int fileno, offset;
	unsigned char buf[BLKSIZE];
	void *p;

	fd = open(fname, O_RDONLY);
	if(fd < 0) {
		perror("open");
		return -1;
	}


	while((count = read(fd, buf, BLKSIZE)) > 0) {
		offset = pino->di_size % BLKSIZE;

		p = &buf;
		for(i = offset; i < count + offset;) {
			
			// NOT ALLIGN BLKSIZE, a port of block
			bn = MIN(BLKSIZE - i, count -i); 
			fileno = pino->di_size / BLKSIZE;

			r = walk_block(pino, fileno, &diskno, 1);
			if(r < 0) {
				printf("walk block error\n");
				return -1;
			}

			/* block not exit in disk */
			if(*diskno == 0) {
				bno = get_free_no(blk_bitmap);
				if(bno < 0) {
					printf("No free inode number\n");
					return -1;
				}

				/* insert alloc block */
				*diskno = bno;
				set_bitmap(blk_bitmap, bno);
			} else
				bno = *diskno;

			memcpy(img_addr + bno * BLKSIZE + (i % BLKSIZE), p, bn);	

			i += bn;
			p += bn;
			pino->di_size += bn;
		}

	}
	close(fd);
	return 0;		
}

static int 
create_file_image(char *name)
{
	int r;

	struct direct_t  *pdir;
	struct inode_t *pinode;
	r = walk_path(name);
	if(r != -1) {
		printf("create file failed\n");
		return -1;	
	}

	r = alloc_file(&pdir);		
	if(r < 0) {
		printf("alloc file failed\n");
		return -1;
	}

	   int ino;
        ino = get_free_no(ino_bitmap);
        if(ino < 0) {
                printf("No free inode number\n");
                return -1;
        }
        set_bitmap(ino_bitmap, ino);
        pinode = &ino_table[ino];

	//init inode 
	pinode->di_size = 0; //
	pinode->di_type = 0; // type 0 File
	
	//attach inode to direct
#ifdef DEBUG
	printf("root inode dir block:%d\n", ino);
#endif
	pdir->ino = ino;
	strncpy(pdir->name, name, 14);
	
	//write file content
	write_file(name, pinode);
 	
	return 0;
}

int main(int argc, char *argv[])
{
	int fd;
	struct super_t sb;
	struct inode_t ino;
	struct direct_t dir;
	ssize_t count;
	void *img_tmp;
	int i = 0;

	if(argc != 3 ) {
		printf("parameter error");
		exit(0);
	}

	fd = open(argv[1], O_RDWR);
	if(fd < 0) {
		perror("open image file error");
		exit(0);
	}

	img_addr = mmap(NULL, IMGSIZE, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
	if(img_addr == MAP_FAILED) {
		perror("map fail");
		return;
	}
	
	img_tmp = img_addr;
/*
 struct super_t {
        unsigned int inodes_count;
        unsigned int blocks_count;
        unsigned int freei_count;
        unsigned int freeb_count;
        unsigned int block_size;
        unsigned short firsti;
        unsigned short magic;
        unsigned int version;
        unsigned int reserv[249];

};
*/

	//1. create supper
	img_tmp += BLKSIZE;
	memset(&sb, 0, BLKSIZE);

	sb.inodes_count =  INOSIZE/ sizeof(struct inode_t);
	sb.blocks_count = IMGSIZE / BLKSIZE;
	sb.freei_count =  INOSIZE/sizeof(struct inode_t);
	sb.freeb_count = IMGSIZE / BLKSIZE;
	sb.block_size = BLKSIZE;
	sb.firsti = ROOT_INODE;
	sb.magic[0]='X';
	sb.magic[1]='Y';
	sb.version = 1;
	memcpy(img_tmp, &sb, BLKSIZE);
	assert(sizeof(struct super_t) == BLKSIZE);	


	// 2. init block bitmap
	img_tmp += BLKSIZE;
	blk_bitmap = img_tmp;
	memset(img_tmp, 0xFF, BLKSIZE);
		
	//3. init inode bitmap
	img_tmp += BLKSIZE;
	ino_bitmap = img_tmp;
	memset(img_tmp, 0xFF, BLKSIZE);

	//4. init inode table
	 img_tmp += BLKSIZE;
	 ino_table = img_tmp;
		
	
	//5. update block bitmap
	for(i = 0; i < 14; i++) {
		set_bitmap(blk_bitmap, i);	
	}
	 

	//6. create root inode 
	/*
	   unsigned short  di_mode;
	   unsigned short  di_nlink;
	   unsigned int    di_size;
	   unsigned short block[12];
	*/
	int blkno;
	memset(&ino, 0 , sizeof(struct inode_t));
	// now alloc block ????
/*
	blkno = get_blk(blk_bitmap)
	if(blkno != 0 ) {
		printf("get_blk failed\n");
		return;
	}
*/
	memcpy(&ino_table[ROOT_INODE], &ino, sizeof(struct inode_t));	
	ino_table[ROOT_INODE].di_size = 0; //
	ino_table[ROOT_INODE].di_type = 1; // type 0 Directory

	assert(sizeof(struct inode_t) == 32);

	//7. update root inode bitmap
	set_bitmap(ino_bitmap, 0);
#ifdef DEBUG
	printf("File system XXX image %s : Pass\n", argv[1]);
#endif
	//8. add a file in the disk image
	create_file_image(argv[2]);	

#ifdef DEBUG
	printf("Create File %s in image file: Pass\n", argv[2]);	
#endif
	//9. finish
	//msync(img_addr, IMGSIZE, MS_SYNC);
	//munmap(img_addr, IMGSIZE);
	close(fd);
#ifdef DEBUG
	printf("Make Image File OK\n");
#endif
	return 0;
}
	

