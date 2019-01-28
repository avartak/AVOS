/* The designated entry point. */
ENTRY(Kstart)

/* Output format */
OUTPUT_FORMAT("binary")

/* Output file */
OUTPUT(kernel.bin)
 
/* Various sections in the final kernel image. */
SECTIONS
{
	/* Kernel loaded at 0xC0100000 */
	. = 0xC0100000;

	/* Multiboot header and code */	 
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}
 
	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}
 
	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}
