/* The designated entry point. */
ENTRY(BootStage1)

/* Output format */
OUTPUT_FORMAT("binary")

/* Various sections in the final kernel image. */
SECTIONS
{
	/* Start of the kernel */
	. = 0x7C00;

	/* Kernel code */	 
	.text :
	{
		*(.boot)
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
