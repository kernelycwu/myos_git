/*
 *  
 */
#include <kern/file.h>
#include <elf.h>
#include <kern/proc.h>
#include <kern/arch.h>
#include <kern/mm.h>
#define BINARYSIZE	128
extern struct proc *current;


int sys_execve(const char *filename, char *const argv[],
                  char *const envp[]) {
	


	struct file  pfile;
	Elf_Ehdr *ehdr;
        Elf_Phdr *phdrs, *phdr;
        int i, j, r;
	unsigned long start;
	unsigned long memsz;
	struct page *pp = 0;
	unsigned long addr;

	unsigned char binary[BINARYSIZE];
	struct proc *p = current;
	void *output = (void *)&binary;

	memset(&binary, 0, 128);
	/* parse the image header */
	file_open(filename, &pfile);

	/* make clear of the user address space */
	for(addr = UTEXT; addr < USTACKTOP; addr += 4096) {
		remove_page(current->pgdir, addr);
	}

	file_read(&pfile, &binary, 128, 0);
	//file_read(&pfile, output, BINARYSIZE, 0);

        ehdr = (Elf_Ehdr *)&binary;
        if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
                        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
                        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
                        ehdr->e_ident[EI_MAG3] != ELFMAG3) {
		print("Not support the binary format\n");
                return -1;
        }

	/* mmap the image file into the address space */
	phdrs = (Elf_Phdr *)(output + ehdr->e_phoff);
	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr = &phdrs[i];
		switch (phdr->p_type) {
			case PT_LOAD:
				start = (unsigned int)phdr->p_vaddr / PAGESIZE  * PAGESIZE ;
				memsz = (phdr->p_memsz + PAGESIZE - 1) / PAGESIZE  * PAGESIZE ;
				int remaining;
                                for(j = 0; j < memsz; j += 4096) {
                                        remaining = phdr->p_filesz - j;
                                        if( remaining >= 0) {   /*Text Data and Bss */
                                                pp = alloc_page(0);
                                                #define MIN(a,b) ((a) > (b) ? (b) : (a))
                                                file_read(&pfile, (void *)PAGE2VA(pp), MIN(4096, remaining),                                                                    phdr->p_offset + j);

                                        } else  /* BSS as a separate segment */
                                                 pp = alloc_page(1);
                                        /* mapping section into the address space */
                                        r = insert_page(p->pgdir, start + j, pp , PTE_U|PTE_W|PTE_P);
                                }


				break;
			default: /* Ignore other PT_* */ break;
		}
	}

	/* alloc prog stack */
        pp = alloc_page(1);
        r = insert_page(p->pgdir, USTACKTOP - 4096, pp , PTE_U|PTE_W|PTE_P);
#ifndef amd64
        p->pregs.esp = USTACKTOP;
        p->pregs.eip =  ehdr->e_entry;
#else
	p->pregs.rsp = USTACKTOP;
        p->pregs.rip =  ehdr->e_entry;
#endif

	run_proc(p);
	return 0;
}
