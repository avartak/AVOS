/* The designated entry point. */
ENTRY(AVBL32)

/* Output format */
OUTPUT_FORMAT("binary")

/* Various sections in the final kernel image. */
SECTIONS
{
	/* Start of the kernel */
	. = 0x8000;

	/* Kernel code */	 
	.text :
	{
		*(.text)
	}
 
	/* Read-only data. */
	.rodata :
	{
		*(.rodata)
	}
 
	/* Read-write data (initialized) */
	.data :
	{
		*(.data)
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss :
	{
		*(COMMON)
		*(.bss)
	}
}
