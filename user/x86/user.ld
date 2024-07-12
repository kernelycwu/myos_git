OUTPUT_FORMAT("elf32-i386", "elf32-i386",
	      "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	. = 0x8048000;

	_start = .; /*user start addr */
	.text 0x8048000 :
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
