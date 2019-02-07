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
	. = 0x100000;

	/* Multiboot header and initial stack */	 
	.start ALIGN(4K) :
	{
        *(.start)
	}
 
	/* Offset of the high memory mapping */
	. += 0xC0000000;

	/* Multiboot header and code */	 
	.text ALIGN(4K) : AT (ADDR(.text) - 0xC0000000)
	{
		*(.text)
	}
 
	/* Read-only data. */
	.rodata ALIGN(4K) : AT (ADDR(.rodata) - 0xC0000000)
	{
		*(.rodata)
	}
 
	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT (ADDR(.data) - 0xC0000000)
	{
		*(.data)
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4K) : AT (ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
	}
}
