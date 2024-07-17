#include <kern/lib.h>
#include <elf.h>

//0xfa
unsigned char testboot_bin[] = {
  0xfa, 0x31, 0xc0, 0x8e, 0xd8, 0x8e, 0xc0, 0x8e, 0xe0, 0x8e, 0xd0, 0x66,
  0x0f, 0x01, 0x16, 0x50, 0x7c, 0x0f, 0x20, 0xc0, 0x0c, 0x01, 0x0f, 0x22,
  0xc0, 0x66, 0xea, 0x21, 0x7c, 0x00, 0x00, 0x08, 0x00, 0xb8, 0x10, 0x00,
  0x00, 0x00, 0x8e, 0xd8, 0x8e, 0xd0, 0x8e, 0xe0, 0x8e, 0xc0, 0xe9, 0xcd,
  0x83, 0x0f, 0x00, 0xeb, 0xfe, 0x8d, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00, 0x17, 0x00, 0x38, 0x7c,
  0x00, 0x00
};
unsigned int testboot_bin_len = 86; 
char buf[4096];
char ttt[4096];


void user_main()
{
	//while(1) ;
	tprintf();

	int pid;
	int j =0;
	 Elf_Ehdr *ehdr;
        Elf_Phdr *phdrs, *phdr;
	void *output;
	int fd;
	int res;
	int i;
	unsigned long memsz;

	/* create cpu */
	create_vcpu();

	/* alloc memory */
	alloc_vm_mem(4*1024*1024);

	/* load a bios-like bootloader */
	mmap_mem(testboot_bin, testboot_bin_len, 0x7c00, 0x7);  

	/* load a os image */
	fd = open("kernel", 1, 1); 
	res = read(fd, &buf, 4096);
	 ehdr = (Elf_Ehdr *) &buf;
        if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
                        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
                        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
                        ehdr->e_ident[EI_MAG3] != ELFMAG3) {
                //print("Not support the binary format\n");
                return ;
        }
	output=&buf;


	 /* mmap the image file into the address space */
	phdrs = (Elf_Phdr *)(output + ehdr->e_phoff);
	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr = &phdrs[i];
		switch (phdr->p_type) {
			case PT_LOAD:

				lseek(fd, phdr->p_offset, 0);
				//memsz = (phdr->p_memsz + 4096 - 1) / 4096  * 4096 ;
				memsz = phdr->p_memsz;
                                int j;
				int remaining;
			#if 1
				for(j = 0; j < memsz; j += 4096) {
					remaining = memsz - j;
					#define MIN(a,b) ((a) > (b) ? (b) : (a))
					
					res = read(fd, &ttt, MIN(4096, remaining));
				
					//res = read(fd, &buf, 4096);
				//	tprintf();
					mmap_mem(&ttt, MIN(4096, remaining), phdr->p_paddr + j, 0x7);
				}
			#endif
				//res = read(fd, &buf, 4096);
				//tprintf();

				break;
			default: /* Ignore other PT_* */ break;
		}
	}


	/* startup my virtual machine */
	run_vm();
        close(fd);
//	run_vm();

	while(1);
	//asm volatile("int $3");
}

