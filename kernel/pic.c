#include <kern/arch.h>
/* primary cmd 0x20 data 0x21 */
#define MASTER_PIC_CMD 	0x20
#define MASTER_PIC_DATA 0x21

/* slave cmd 0xA0, data 0xA1 */
#define SLAVE_PIC_CMD 	0xA0
#define SLAVE_PIC_DATA  0xA1
void send_eoi(unsigned char irq)
{
	/*OCW2 Commands */
	if(irq >= 8)
		outb(SLAVE_PIC_CMD, 0x20);
	outb(MASTER_PIC_CMD, 0x20);
}
/* Interrupt Mask Register */
void clear_imr(unsigned char irq)
{
	unsigned char master_imr;
	unsigned char slave_imr;
	unsigned char mask;

	master_imr = inb(MASTER_PIC_DATA);
	slave_imr = inb(SLAVE_PIC_DATA);

	if(irq < 8) {
		mask = ~(1 << irq);
		outb(MASTER_PIC_DATA, master_imr & mask);
	}
	else {
		mask = ~(1 << (irq - 8));
		outb(SLAVE_PIC_DATA, slave_imr & mask);
	}	
	
}
void set_imr(unsigned char irq)
{
	unsigned char master_imr;
        unsigned char slave_imr;
	unsigned char mask;

        master_imr = inb(MASTER_PIC_DATA);
        slave_imr = inb(SLAVE_PIC_DATA);
        if(irq < 8) {
                mask = (1 << irq);
                outb(MASTER_PIC_DATA, master_imr | mask);
        }
        else {
                mask = (1 << (irq - 8));
                outb(SLAVE_PIC_DATA, slave_imr | mask);
        }

}

void init_pic()
{
	 outb(MASTER_PIC_DATA, 0xFF);
         outb(SLAVE_PIC_DATA, 0xFF);

	/* send icw 1 
	   Bit 4 - Initialization bit. Set to 1
 	   Bit 0 - Set to 1 so we can sent ICW 4
	*/
	outb(MASTER_PIC_CMD, 0x11);
	outb(SLAVE_PIC_CMD, 0x11);
	/* send icw 2  x86 Interrupt Vector Table (IVT) table base address
 	 *  IRQs 32-47 :PRIMARY:IRQ 0..7 <---->32..39; SLAVE: IRQ's 8..15 <---> 40
 	 */ 
	outb(MASTER_PIC_DATA, 0x20);
	outb(SLAVE_PIC_DATA, 0x28);
	/* send icw 3 
	 * The 80x86 architecture uses IRQ line 2 to connect the master PIC to the slave PIC
	 */
	 outb(MASTER_PIC_DATA, 0x4);
         outb(SLAVE_PIC_DATA, 0x2);
	
	/* send icw 4 Set x86 mode */
	 outb(MASTER_PIC_DATA, 0x3);
         outb(SLAVE_PIC_DATA, 0x1);

	  outb(MASTER_PIC_CMD, 0x68);             /* clear specific mask */
        outb(MASTER_PIC_CMD, 0x0a);             /* read IRR by default */

        outb(SLAVE_PIC_CMD, 0x68);               /* OCW3 */
        outb(SLAVE_PIC_CMD, 0x0a);               /* OCW3 */

	/* All done. Null out the data registers */
	 outb(MASTER_PIC_DATA, 0xFF);
         outb(SLAVE_PIC_DATA, 0xFF);

 	clear_imr(2); 	
 	clear_imr(1); 	
 	clear_imr(0); 	
}
