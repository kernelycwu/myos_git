#define EI_NIDENT       16
#define PT_LOAD         1
#define ELFMAG0         0x7f            /* EI_MAG */
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'
#define EI_MAG0         0               /* e_ident[] indexes */
#define EI_MAG1         1
#define EI_MAG2         2
#define EI_MAG3         3


#ifndef amd64
typedef struct elf32_phdr{
  unsigned int    p_type;
  unsigned int     p_offset;
  unsigned int    p_vaddr;
  unsigned int    p_paddr;
  unsigned int    p_filesz;
  unsigned int    p_memsz;
  unsigned int    p_flags;
  unsigned int    p_align;
} Elf32_Phdr;


typedef struct elf32_hdr{
  unsigned char e_ident[EI_NIDENT];
  unsigned short    e_type;
  unsigned short    e_machine;
  unsigned int    e_version;
  unsigned int    e_entry;  /* Entry point */
  unsigned int     e_phoff;
  unsigned int     e_shoff;
  unsigned int    e_flags;
  unsigned short    e_ehsize;
  unsigned short    e_phentsize;
  unsigned short    e_phnum;
  unsigned short    e_shentsize;
  unsigned short    e_shnum;
  unsigned short    e_shstrndx;
} Elf32_Ehdr;
#else
typedef struct elf64_hdr {
  unsigned char e_ident[EI_NIDENT];     /* ELF "magic number" */
  unsigned short e_type;
  unsigned short e_machine;
  unsigned int e_version;
  unsigned long long e_entry;           /* Entry point virtual address */
  unsigned long long e_phoff;            /* Program header table file offset */
  unsigned long long e_shoff;            /* Section header table file offset */
  unsigned int e_flags;
  unsigned short e_ehsize;
  unsigned short e_phentsize;
  unsigned short e_phnum;
  unsigned short e_shentsize;
  unsigned short e_shnum;
  unsigned short e_shstrndx;
} Elf64_Ehdr;

typedef struct elf64_phdr {
  unsigned int p_type;
  unsigned int p_flags;
  unsigned long long p_offset;           /* Segment file offset */
  unsigned long long p_vaddr;           /* Segment virtual address */
  unsigned long long p_paddr;           /* Segment physical address */
  unsigned long long p_filesz;         /* Segment size in file */
  unsigned long long p_memsz;          /* Segment size in memory */
  unsigned long long p_align;          /* Segment alignment, file & memory */
} Elf64_Phdr;
#endif


inline void
outb(unsigned short port, unsigned char value)
{
        asm volatile("outb %0, %1"
                        :
                        :"a"(value), "d"(port)
                        :);
}
static inline unsigned char
inb(unsigned short port)
{
        unsigned char value;
        asm volatile( "inb %1, %0"
                        : "=a" (value)
                        : "d"(port)
                        : );
        return value;
}
static inline void
insw(unsigned short port, void *buf, unsigned short count)
{
        asm volatile("cld\t\n rep insw"
                    : "=D" (buf)
                    :"D" (buf), "d" (port), "c"(count)
                    : "memory");
}

void wait()
{
	 while((inb(0x1f7) & 0x80) != 0) ;
/*BugFix:*/
//         while(inb(0x1f7) & 0x80 != 0)  ;
}
void diskcpy(void *dest,  unsigned int lba, unsigned int count)
{
        outb(0x1F6, 0xE0| ((lba >> 24) & 0x0F));
        outb(0x1F1, 0x00);
        outb(0x1F2, (unsigned char) count);
        outb(0x1F3, (unsigned char) lba);
        outb(0x1F4, (unsigned char)(lba >> 8));
        outb(0x1F5, (unsigned char)(lba >> 16));
        outb(0x1F7, 0x20);
        
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
/* The number of columns.  */
#define COLUMNS                 80
/* The number of lines.  */
#define LINES                   24
/* The attribute of an character.  */
#define ATTRIBUTE               7

static int xpos;
/* Save the Y position.  */
static int ypos;
/* Point to the video memory.  */
static volatile unsigned char *video;
#define VIDEO                   0x000B8000
/* Clear the screen and initialize VIDEO, XPOS and YPOS.  */
void
cls (void)
{
  int i;

  video = (unsigned char *) VIDEO;

  for (i = 0; i < COLUMNS * LINES * 2; i++)
    *(video + i) = 0;

  xpos = 0;
  ypos = 0;
}
void main(void)
{
	//cls();


	int i;
	unsigned int m,k;
	void *output = (void *)0x80000;


	diskcpy(output,  1, 1);
#ifndef amd64

	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdrs, *phdr;

	ehdr = (Elf32_Ehdr *) output;
	phdrs = (Elf32_Phdr *)(output + ehdr->e_phoff);
#else
	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdrs, *phdr;

	ehdr = (Elf64_Ehdr *) output;
	phdrs = (Elf64_Phdr *)(output + ehdr->e_phoff);

#endif
	if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
			ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
			ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
			ehdr->e_ident[EI_MAG3] != ELFMAG3) {

		return;
	}

	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr = &phdrs[i];
		switch (phdr->p_type) {
			case PT_LOAD:
				m = phdr->p_offset/512;
				k = (phdr->p_filesz+511)/512;
				diskcpy((void *)phdr->p_paddr, m+1, k);
				
				break;
			default: /* Ignore other PT_* */ break;
		}
	}

	void (*fn)(void);
	fn = (void *)ehdr->e_entry;
	fn();
}
