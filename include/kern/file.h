#ifndef FILE_H
#define FILE_H
#include "fs.h"
enum {
	FDEUNKNOWN,
	FDCONS,
	FDFILE,
};
struct file {
	int refcount;
	int offset;
	struct inode_t *inode;
	int type;
};
int file_open(const char *fname, struct file* file);
int file_read(struct file *file, void *buf, unsigned int count, int offset);
#endif
