/* The designated entry point. */
ENTRY(Kstart)

/* Output format */
OUTPUT_FORMAT("binary")

/* Output file */
OUTPUT(kernel.bin)
 
/* Various sections in the final kernel image. */
SECTIONS
{
	/* Start of the kernel */
	. = 0xC0100000;

	/* Kernel code */	 
	.text ALIGN(4K) :
	{
		*(.text)
	}
 
	/* Read-only data. */
	.rodata ALIGN(4K) :
	{
		*(.rodata)
	}
 
	/* Read-write data (initialized) */
	.data ALIGN(4K) :
	{
		*(.data)
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4K) :
	{
		*(COMMON)
		*(.bss)
	}
}
