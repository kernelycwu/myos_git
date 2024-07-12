OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64",
	      "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
	. = 0x100000;
	_start = .; /*kernel start addr */
	.head : {
		kernel/amd64/head.o (.text .data .bss)
	}

	. = 0x200000;
	entry64 = .;
	.text 0xffffffff80200000 : AT(0x200000) 
	{
		*(EXCLUDE_FILE(kernel/amd64/head.o) .text .stub .text.* .gnu.linkonce.t.*)
	} 
	PROVIDE(etext = .);

	.data ALIGN (4K) :AT(ADDR (.data) - 0xffffffff80000000)
	{
		 *(EXCLUDE_FILE(kernel/amd64/head.o) .data)	
	}
	_edata = .; 
	PROVIDE (edata = .);

	.bss :AT(ADDR (.bss) - 0xffffffff80000000)
	{
		*(EXCLUDE_FILE(kernel/amd64/head.o) .bss)
	}
	_end = .; /* kernel end addr */
	PROVIDE (end = .);

}
