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

typedef Elf32_Phdr  Elf_Phdr;
typedef Elf32_Ehdr  Elf_Ehdr;
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
typedef Elf64_Phdr  Elf_Phdr;
typedef Elf64_Ehdr  Elf_Ehdr;

#endif
