/* The designated entry point. */
ENTRY(Start)

/* Various sections in the final kernel image. */
SECTIONS
{
	/* Start of the kernel */
	__kernel_load_addr   = 1M;
	__higher_half_offset = 0xC0000000;
	. = __higher_half_offset + __kernel_load_addr;

	/* Kernel code */	 
	.text ALIGN(4K) : AT (ADDR (.text) - __higher_half_offset)
	{
		__load_addr   = . - __higher_half_offset;
		__header_addr = . - __higher_half_offset;
		*(.multiboot)
		__entry_addr  = . - __higher_half_offset;
		*(.text)
	}
 
	/* Read-only data. */
	.rodata ALIGN(4K) : AT (ADDR (.rodata) - __higher_half_offset)
	{
		*(.rodata)
	}
 
	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT (ADDR (.data) - __higher_half_offset)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4K) : AT (ADDR (.bss) - __higher_half_offset)
	{
		__load_end_addr = . - __higher_half_offset; 
		*(COMMON)
		*(.bss)
	}
 	__bss_end_addr = . - __higher_half_offset;
}
