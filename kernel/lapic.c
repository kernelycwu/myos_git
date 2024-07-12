#include <kern/arch.h>
#include <kern/mm.h>
struct mp_floating_pointer_structure {
	char signature[4];
	unsigned int configuration_table;
	unsigned char  length; // In 16 bytes (e.g. 1 = 16 bytes, 2 = 32 bytes)
	unsigned char  mp_specification_revision;
	unsigned char  checksum; // This value should make all bytes in the table equal 0 when added together
	unsigned char  default_configuration; // If this is not zero then configuration_table should be 
	// ignored and a default configuration should be loaded instead
	unsigned int  features; // If bit 7 is then the IMCR is present and PIC mode is being used, otherwise 
	//                            // virtual wire mode is; all other bits are reserved
};

struct mp_configuration_table {
	char signature[4]; // "PCMP"
	unsigned short length;
	unsigned char mp_specification_revision;
	unsigned char checksum; // Again, the byte should be all bytes in the table add up to 0
	char oem_id[8];
	char product_id[12];
	unsigned int oem_table;
	unsigned short oem_table_size;
	unsigned short entry_count; // This value represents how many entries are following this table
	unsigned int lapic_address; // This is the memory mapped address of the local APICs 
	unsigned short extended_table_length;
	unsigned char extended_table_checksum;
	unsigned char reserved;
};


struct entry_processor {
	unsigned char type; // Always 0
	unsigned char local_apic_id;
	unsigned char local_apic_version;
	unsigned char flags; // If bit 0 is clear then the processor must be ignored
	// If bit 1 is set then the processor is the bootstrap processor
	unsigned int signature;
	unsigned int feature_flags;
	unsigned int reserved[2];
};


struct entry_io_apic {
	unsigned char type; // Always 2
	unsigned char id;
	unsigned char version;
	unsigned char flags; // If bit 0 is set then the entry should be ignored
	unsigned int address; // The memory mapped address of the IO APIC is memory
};

struct mp_floating_pointer_structure *mpfp = 0;
struct mp_configuration_table *mpct = 0;
struct entry_processor *ep = 0;
struct entry_io_apic *eia = 0;

int find_smp_config(unsigned long addr, unsigned long size)
{
	unsigned int i = 0;
	unsigned char *start = (unsigned char *)KPA2VA(addr); 

	while(i < size ) {
		if(start[0] == '_' && start[1] == 'M' 
			&& start[2] == 'P' && start[3] == '_' ) {
			mpfp = (struct mp_floating_pointer_structure *)start;
			return 1;
		}

		start += 4;
		i += 4;		
	}
	return 0;
}

#define MAXCPUS 8
unsigned char lapic_ids[MAXCPUS];
static int numcores = 0;
unsigned long spa = 0;

void init_mp_config()
{
	int i=0;
	/*
	 * 1) Scan the bottom 1K for a signature
	 * 2) Scan the top 1K of base RAM
	 * 3) Scan the 64K of bios
	 */
	if(find_smp_config(0x0, 0x400) ||
			find_smp_config(639 * 0x400, 0x400) ||
			find_smp_config(0xF0000, 0x10000)) {

		print("find smp config success:%x,%x, %x, %x\n", mpfp->signature[0], mpfp->signature[1], mpfp->signature[2], mpfp->signature[3]);
		print("mpfp:%x\n", mpfp);
		print("mpfp table:%x\n", mpfp->configuration_table);
		mpct = (struct mp_configuration_table *)KPA2VA(mpfp->configuration_table);
		
		print("find smp config success:%x,%x, %x, %x\n", mpct->signature[0], mpct->signature[1], mpct->signature[2], mpct->signature[3]);
		print("lapic addr:%x\n",mpct->lapic_address);
		print("lapic entry:%d\n",mpct->entry_count);
		//ep = mpct + sizeof(struct mp_configuration_table);
		unsigned char *pp = (unsigned char *)mpct+ sizeof(struct mp_configuration_table);

		for(i = 0; i < mpct->entry_count; i++) {
			switch(*pp) {
				case 0:
					ep = (struct entry_processor*)pp;	
					print("local_apic_id:%d,%d\n",ep->local_apic_id, sizeof(struct entry_processor));
					lapic_ids[numcores] = ep->local_apic_id;	
					pp += sizeof(struct entry_processor); 
					numcores++;				
					break;
				case 2:	
					eia = (struct entry_io_apic*)pp;
					print("\n%d, %x\n",eia->id, eia->address);
					pp += sizeof(struct entry_io_apic);
					break;
				default:
					//print("unkown type:%d ", *pp);
					pp += 8;
					break;						
			}
		}
							
		return;		
	}
	
}

unsigned int check_apic ()
{
	unsigned int apic_flag = 0;
	asm volatile("movl $01, %%eax \t\n"
		     "cpuid"
		    :"=d" (apic_flag)
		);
	print("apic_flag:%x\n", apic_flag & 0x200);
	
	return apic_flag;
}

unsigned int read_lapic(unsigned int msr)
{
	unsigned int low, high;
	asm volatile("rdmsr"
		      : "=a" (low), "=d" (high)
		      : "c" (msr));
		print("lapic addr:%x\n", low);
	return low;	
}

extern void* kpgdir;
volatile void * lapic_addr;
//extern unsigned char kpgdir[];

unsigned int enable_lapic()
{
/*
	int r;
	unsigned long *pte;

	unsigned int paddr = read_lapic(0x1B) & 0xFFFFF000;
	void * va = (void *)paddr;
	lapic_addr = va;

        r = walk_page(kpgdir, va, 1, &pte);
        if(r < 0 ) {
                print("walk_page error\n");
                return r;
        }

        *pte = paddr | PTE_U|PTE_W|PTE_P;
	tlb_invalid(va);
        
	unsigned int spur = *(unsigned int *)(va + 0xF0);
	*(unsigned int *)(va + 0xF0) = spur | 0x100;
	print("spur intr :%x\n", spur);
*/
	unsigned int paddr = read_lapic(0x1B) & 0xFFFFF000;
	int r;
	unsigned long *pte;
	void *va = 0xffffffff00000000 | paddr;

        lapic_addr = va;
        r = walk_page(kpgdir, va, 1, &pte);
        if(r < 0 ) {
                print("walk_page error\n");
                return r;
        }
        *pte = paddr | PTE_U|PTE_W|PTE_P;

        tlb_invalid(va);
        
	unsigned int spur = *(unsigned int *)(va + 0xF0);
	*(unsigned int *)(va + 0xF0) = spur | 0x100;
	print("spur intr :%lx\n", spur);

}

unsigned char bspdone = 0;      //bootcpu done flag;
volatile unsigned char  aprunning = 0; //running cpu count;
void startup_smp()
{
	int i, j;
	unsigned int p;
	unsigned char  bspid; // bootcpu id

	// get the BSP's Local APIC ID
	asm volatile ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(bspid) : : );
	print ("bspid:%d\n", bspid);

	for(i=0; i< numcores; i++) {
#if 0
		outb(0x70, 0xF);  // offset 0xF is shutdown code
		outb(0x70+1, 0x0A);
		unsigned short *wrv;
		wrv = (unsigned short *)((0x40 << 4 | 0x67)| 0xc0000000);  // Warm reset vector
		wrv[0] = 0;
		wrv[1] = 0x8000 >> 4;
#endif
		if(lapic_ids[i] == bspid)
			continue;

		*(volatile unsigned int *)(lapic_addr + 0x310) =  i << 24;

		*(volatile unsigned int *)(lapic_addr + 0x300) = 0x00C500;

		*(volatile unsigned int *)(lapic_addr + 0x300) = 0x008500;

		for(j=0; j<2; j++) {
			*(volatile unsigned int *)(lapic_addr + 0x310) =  i << 24;
			p = *(volatile unsigned int *)(lapic_addr + 0x0020);
			*(volatile unsigned int *)(lapic_addr + 0x300) = 0x00000600|0x8;
			p = *(volatile unsigned int *)(lapic_addr + 0x0020);
		}
	}

/*	
	   unsigned char *va = (unsigned char *) KPA2VA(0x8000);
	   for(i = 0; i < 128; i++)
	   print("%x ",va[i]);
*/		
	 
}

void commence_smp()
{
	// release spinlock 
	bspdone = 1;
}

void init_smp()
{
	/*1.  searches for the MP Floating Pointer structure. */
	/*2.  find the MP Configuration Table*/	
	/*3.  how many processors and IO APICs are in the system.*/
	init_mp_config();
	/*4.  initializes the bootstrap processor's local APIC*/
	check_apic();
	//read_apic(0x1B);
	enable_lapic();	
	/*5.  OS sends Startup IPIs to each of the other processors with the address of trampoline code.*/
	startup_smp();
	/*6. The trampoline code initializes the AP's to protected mode and enters the OS code to being further initialization.*/
	/*7.  the BSP can initialize the IO APIC into Symmetric IO mode, to allow the AP's to begin to handle interrupts.*/
	/*8. The OS continues further initialization, using locking primitives as necessary.*/
	commence_smp();	
}

// this C code can be anywhere you want it, no relocation needed
void ap_startup(int apicid) {

	// do what you want to do on the AP
	print("Starting xxxx cpu:%d\n", apicid);
/*
	unsigned int test = 0x01;
	unsigned int nr = 3;
	unsigned ret = 0;
	asm volatile("lock bts %2, %1\t\n"
		      "movl %%edx, %0"
		 	: "=r" (ret)
			: "d" (test), "r" (nr));
	print("ret=%d\n", ret);
*/
	while(1);
}
