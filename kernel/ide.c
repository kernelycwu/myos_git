#include <kern/arch.h>
void wait()
{
        while((inb(0x1f7) & 0x80) != 0)
               ;

}
void readblk(void *dest,  unsigned int lba, unsigned int count)
{
 //       outb(0x1F6, 0xE0| ((lba >> 24) & 0x0F)); //start 
        outb(0x1F1, 0x00);
        outb(0x1F2, (unsigned char) count); //count
        outb(0x1F3, (unsigned char) lba);
        outb(0x1F4, (unsigned char)(lba >> 8));
        outb(0x1F5, (unsigned char)(lba >> 16));
        outb(0x1F6, 0xF0| ((lba >> 24) & 0x0F)); //start 
        outb(0x1F7, 0x20); //read command

        while(count != 0) {
                wait();
               insw(0x1F0, dest, 256);
                inb(0x1f0);
                inb(0x1f0);
                inb(0x1f0);
                inb(0x1f0);
		wait();
                dest += 512;
                count--;
        }
  
}
void writeblk(void *buf, unsigned int lba, unsigned int count)
{
	//outb(0x1F6, 0xE0| ((lba >> 24) & 0x0F)); //start 
	outb(0x1F6, 0xF0| ((lba >> 24) & 0x0F)); //start 
	outb(0x1F1, 0x00);
	outb(0x1F2, (unsigned char) count); //count
	outb(0x1F3, (unsigned char) lba);
	outb(0x1F4, (unsigned char)(lba >> 8));
	outb(0x1F5, (unsigned char)(lba >> 16));
	outb(0x1F7, 0x30); //write command

	while(count != 0) {
		wait();
		outsw(0x1F0, buf, 256);
		inb(0x1f0);
		inb(0x1f0);
		inb(0x1f0);
		inb(0x1f0);
		wait(); 
		buf += 512;
		count--;
	}
	
}
