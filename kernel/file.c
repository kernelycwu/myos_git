#include <kern/file.h>
#include <kern/proc.h>
#include <kern/fs.h>
#include <kern/lib.h>
#include <kern/console.h>
#define MAXFILE 256
struct file filetable[MAXFILE];
extern struct proc *current;

void init_files()
{
	memset(&filetable, 0, sizeof(struct file) *MAXFILE);
}
static int  allocfd(void)
{
	int i;
	for(i = 0; i < MAXOPENFILE; i++) {
		if(current->fdtable[i] == 0)
			return i;
	}	
	return -1;
}
static struct file* allocfile(void)
{
	int i;

	for(i = 0; i < MAXFILE; i++) {
		if(filetable[i].refcount == 0) {
			memset(&filetable[i], 0, sizeof(struct file));
			return &filetable[i];
		}
	}
	return NULL;	
}

int file_open(const char *fname, struct file* file)
{
	int r; 
	struct inode_t *inode;
	/* if file != NULL */

	r = walk_path(fname, &inode);
	if(r < 0)
		return r;

	file->inode = inode;
	return 0;
}

#define MIN(a,b) ((a) > (b) ? (b) : (a))
 
int file_read(struct file *file, void *buf, unsigned int count, int offset)
{
	int n, r;
	unsigned short *diskno;
	int filepos, fileno;
	int coffset, remain, mincount;

	for (n = 0; n < count; n += mincount) {
		filepos = offset + n;

		fileno = filepos / BLKSIZE;
		r = walk_block(file->inode, fileno, &diskno, 0);

                coffset =  filepos  % BLKSIZE;
                remain  =  BLKSIZE - coffset;

		/* to read bytes is less than  remaining bytes */
                mincount = MIN(remain, count);
		if(*diskno != 0) { 
			memcpy(buf, bread(*diskno) + coffset, mincount);
		}
		else
			return -1;

		file->offset += mincount;		
		//print("#%s", buf);	
		buf += mincount;
		//n += mincount;				
	}
	return n;	
}

int file_write(struct file *file, void *buf, unsigned int count)
{
	int i,r;
        unsigned short *diskno;
	int bno;
	int curpos, curcount, mincount;
	int offset = file->offset;

        for(i = offset; i <  offset + count;) {
               r = walk_block(file->inode, i / BLKSIZE, &diskno, 1);

		curpos = i % BLKSIZE;
		curcount = BLKSIZE - curpos;

		/* To write bytes is less than  remaining bytes */
		mincount = MIN(curcount, count);

		if(*diskno == 0) { /* alloc disk block  */
			bno = get_blk();
			if(bno < 0 )
				return -1;
			*diskno = bno;
		}

		/* write page cache */
		void *pbuf = bread(*diskno);
		//memcpy(pbuf + curpos, buf, curcount);
		memcpy(pbuf + curpos, buf, mincount);

		/* flush page cache into disk */
		writeblk(pbuf, *diskno * 8, 8);
		/*	
		buf += curcount;
		i += curcount;
		*/
		/* set position offset */
		file->offset += mincount;

		buf += mincount;
                i += mincount;		
		
	}	
	return count;
}
int file_close(struct file *pfile)
{
	return 0;
}


int sys_opencons()
{

	struct file* pcons = NULL;
        int fd;

        print("open console\n");
	 if(fd = allocfd() < 0)
                return fd;

       if (!(pcons = allocfile()))
		return -1;

        pcons->refcount++;
	pcons->type = FDCONS;
        current->fdtable[fd] = pcons;

        return fd;
	
}

static int  
cons_read(struct file *file, void *buf, unsigned int count, int offset)
{
	int i = 0;
	char ch;
	while(i < count) {
		ch = get_kbdata();
		if(ch == -1) 
			break; 
		*(unsigned char*)buf++ = ch;
		i++;
	}
	return i;	
}

static int
cons_write(struct file *file, void *buf, unsigned int count)
{
	 int i = 0;
        char ch;
        while(i < count) {
		myputchar(*(unsigned char*)buf++);	
		i++;	
	}
	return i;
} 

int sys_open(const char *fname, int flags, int mode)
{

	struct file* pfile;
	int r, fd;
	
	print("syscall: sys_open:%s\n", fname);
	if(fd = allocfd() < 0)
		return fd;
	

	pfile = allocfile();
	if(!pfile)
		return -1;


	print("sys_open:%d\n", fd);	
	if(r = file_open(fname, pfile) < 0)	
		return r;

	pfile->offset = 0;
	pfile->refcount++;
	pfile->type = FDFILE;

	current->fdtable[fd] = pfile;

	return fd;
}

int sys_read(int fd, void *buf, unsigned int count)
{

	struct file* pfile;
	int offset;

	pfile = current->fdtable[fd];
	offset = pfile->offset;
	//print("read:%lx, count=%d\n", offset, count);
	switch(pfile->type) {	
		case FDFILE: 
			return	file_read(pfile, buf, count, offset);
		case FDCONS:
			return  cons_read(pfile, buf, count, offset);	
		default:
			print("unknown file type\n");
			return -1;			
	}
}

int sys_write(int fd, void *buf, unsigned int count)
{
	struct file* pfile;

	pfile = current->fdtable[fd];
	switch(pfile->type) {
		case FDFILE:
			return	file_write(pfile, buf, count);
		case FDCONS:
			return  cons_write(pfile, buf, count);	
		default:
			print("sys_write: unknown file type\n");
			return -1;
	}
}

int sys_close(int fd)
{
	struct file* pfile = NULL;

	if(fd < 0 || fd > MAXOPENFILE -1)
		return -1;

	pfile = current->fdtable[fd];
	if(!pfile)
		return -1;
	pfile->refcount--;
	
	if(pfile->refcount == 0) {
		current->fdtable[fd] = NULL;
		pfile->offset = 0;
		pfile->inode = NULL;
		pfile->type = FDEUNKNOWN;
	}
	return 0;
}

int sys_lseek(int fd, int offset, int whence) {
	struct file *pfile = NULL;

	print("sys_lseek:%lx\n", offset);
#if 1
	pfile = current->fdtable[fd];
	if(!pfile)
		return -1;
	/* TODO parameter whence */
//	if(offset > pfile->inode->di_size)
//		return -1;
	print("file size:%d\n", pfile->inode->di_size);
	pfile->offset = offset;	

	return pfile->offset;

#endif
//	return 0;
}

