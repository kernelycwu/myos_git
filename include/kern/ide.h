#ifndef __IDE_H
void readblk(void *dest,  unsigned int lba, unsigned int count);
void writeblk(void *buf, unsigned int lba, unsigned int count);
#endif
