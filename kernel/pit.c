#include <kern/arch.h>
/*
 * I/O port     Usage
 * 0x40         Channel 0 data port (read/write)
 * 0x41         Channel 1 data port (read/write)
 * 0x42         Channel 2 data port (read/write)
 * 0x43         Mode/Command register (write only, a read is ignored)
 **/
#define PIT_CH0_DATA 0x40
#define PIT_CH1_DATA 0x41
#define PIT_CH2_DATA 0x42
#define PIT_MOD_CMD      0x43
//#define HZ 	100
#define HZ 	1
#define LATCH  (1193182/HZ)
//extern volatile  unsigned long tick;
void init_pit()
{
	/* binary, mode 2, LSB/MSB, ch 0 */
	outb(PIT_MOD_CMD, 0x34);
	outb(PIT_CH0_DATA, LATCH & 0xFF);
	outb(PIT_CH0_DATA, (LATCH >> 8) & 0xFF);
//	tick = 0;
//	clear_imr(0);	
	
}
#if 0
void msdelay(unsigned int ms)
{
	unsigned long start = tick;
	while(1) {
		print("tick:%d\n", tick);
		if(start + ms < tick)
			break;
	}
		
}
#endif
