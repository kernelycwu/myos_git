#ifndef __FS_H
#define __FS_H
#define BLKSIZE (4096)
struct super_t {
	unsigned int inodes_count;
	unsigned int blocks_count;
	unsigned int freei_count;
	unsigned int freeb_count;
	unsigned int block_size;
	unsigned short firsti;
	unsigned char magic[2];
	unsigned int version;
	unsigned int reserv[1024-7];
	
};

struct inode_t {
        unsigned int    di_size;
        unsigned short  di_mode;
        unsigned short  di_type;
        unsigned short  di_nlink;
        unsigned short block[11];
};

struct direct_t{
	unsigned short ino;
	char name[14];
};

//int walk_path(const char *fname, struct inode_t **inode);
int walk_block(struct inode_t *p, unsigned int fileno, unsigned short **diskno, int alloc);
int get_blk();
void* bread(int block);
#endif
