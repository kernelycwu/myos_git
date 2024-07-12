OUTPUT_FORMAT("elf32-i386", "elf32-i386",
	      "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	. = 0x100000;

	_start = .; /*kernel start addr */
	.text 0xC0100000 : AT(0x100000) 
	{
		*(.text)
	        *(.rodata) 
	} 
	PROVIDE(etext = .);

	.data ALIGN (4K) :AT(ADDR (.data) - 0xc0000000)
	{	

		*(.data)
	}
	_edata = .; 
	PROVIDE (edata = .);

	.bss :AT(ADDR (.bss) - 0xc0000000)
	{
		*(.bss)
		*(COMMON)
	}
	_end = .; /* kernel end addr */
	PROVIDE (end = .);

}
