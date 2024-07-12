#ifndef MKXXX_H
#define MKXXX_H
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
	unsigned int	di_size;
	unsigned short	di_mode;
	unsigned short	di_type;
	unsigned short	di_nlink;
	unsigned short block[11];
};

struct direct_t{
	unsigned short ino;
	char name[14];
};
#endif
