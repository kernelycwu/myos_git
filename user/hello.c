#include <kern/lib.h>
#include <elf.h>

#if 1
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
#if 1
	int pid;
	int j =0;
	 Elf_Ehdr *ehdr;
        Elf_Phdr *phdrs, *phdr;
	void *output;
#if 0
	pid = fork();
	if(pid == 0) {
		//j=1;
		tprintf();
		while(1);
	}
#endif
#if 0
	int fd;
	int res;
	int i;
	unsigned long memsz;
	create_vcpu();

	alloc_vm_mem(4*1024*1024);
	mmap_mem(testboot_bin, testboot_bin_len, 0x7c00, 0x7);  


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



//	run_vm();
			//tprintf();
//	memset(&buf, 'c', 10);
	//buf[10]='\0';
//	write(fd, &buf, 4096);
//	res = read(fd, &buf, 4096);
        close(fd);
//	run_vm();
#endif
#endif
	while(1);
	//asm volatile("int $3");
}
#endif
#if 0
const unsigned char code[] = {
        0xba, 0xf8, 0x03, /* mov $0x3f8, %dx */
        0xb0, 'H',       /* mov $'H', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'e',       /* mov $'e', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'l',       /* mov $'l', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'l',       /* mov $'l', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'o',       /* mov $'o', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, ',',       /* mov $',', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, ' ',       /* mov $' ', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'w',       /* mov $'w', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'o',       /* mov $'o', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'r',       /* mov $'r', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'l',       /* mov $'l', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, 'd',       /* mov $'d', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, '!',       /* mov $'!', %al */
	0xee,             /* out %al, (%dx) */
        0xb0, '\n',       /* mov $'\n', %al */
        0xee,             /* out %al, (%dx) */
        0xf4,             /* hlt */
    };
/*
unsigned char testboot_bin[] = {
  0xfa, 0x31, 0xc0, 0x8e, 0xd8, 0x8e, 0xc0, 0x8e, 0xe0, 0x8e, 0xd0, 0x66,
  0x0f, 0x01, 0x16, 0x50, 0x7c, 0x0f, 0x20, 0xc0, 0x0c, 0x01, 0x0f, 0x22,
  0xc0, 0x66, 0xea, 0x21, 0x7c, 0x00, 0x00, 0x08, 0x00, 0xb8, 0x10, 0x00,
  0x00, 0x00, 0x8e, 0xd8, 0x8e, 0xd0, 0x8e, 0xe0, 0x8e, 0xc0, 0xf4, 0xe9,
  0xcc, 0x83, 0x0f, 0x00, 0xeb, 0xfe, 0x66, 0x90, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00, 0x17, 0x00, 0x38, 0x7c,
  0x00, 0x00
};
*/
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
void user_main()
{
/*
	int pid;
	 pid = fork();
        if(pid == 0) {
		tprintf();
		user_exit();
		//while(1) {tprintf(); yield();}
	}
*/
	tprintf();
	create_vcpu();
	mmap_mem(testboot_bin, testboot_bin_len, 0x7c00, 0x7);	
//	mmap_mem(code, sizeof(code), 0, 0x7);
	run_vm();
	while(1) {;}
}
#endif

#if 0 
int getline(unsigned char *buf, int n, int fd)
{

        char ch = 0;
        int i = 0;
        int r;
        unsigned char *cur = buf;

        while(ch != '\n' &&  i < n) {
                r = read(fd, &ch, 1);
                if(r == 0)
                        yield();
                else {
                        *cur++ = ch;
                        i++;
                }
        }
        return i;

}


void user_main()
{
        char buf[512];
        int pid;
        char ch;
        int r;
        int i = 0;
        int fd;
        fd = opencons();
        while(1) {


                write(fd, ">", 1);
                getline((unsigned char *)&buf[0], 512, fd);
                pid = fork();
                if(pid == 0) {
			//tprintf();
			//while(1) yield();	
                        execve("hello", 0, 0);
                } 
                waitpid();
        }
        while(1) ;
}
#endif      