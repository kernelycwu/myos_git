OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64",
	      "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS
{
	. = 0x80000000;

	_start = .; /*user start addr */
	.text 0x80000000 :
	{
		*(.text)
	        *(.rodata) 
	} 

	.data ALIGN (4K) :AT(ADDR (.data)) 
	{	

		*(.data)
	}

	.bss ALIGN (4K):
	{
		*(.bss)
		*(COMMON)
	}

}
