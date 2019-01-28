; Kstart is the start of our kernel

; We assume the following to be done by the boot loader : 
; - System in protected mode (PE bit : enabled in protected mode is set)
; - The A20 gate is enabled
; - The code segment (CS) is a valid protected mode code segment with an offset of 0 and limit of 0xFFFFFFFF 
; - Segments DS, ES, FS, GS, SS are valid protected segments with an offset of 0 and limit of 0xFFFFFFFF
; - Kernel is placed at 0x100000
; - Interrupt flag (IF bit 9) in EFLAGS is cleared i.e. maskable interrupts are disabled
; - Virtual 8086 mode is disabled i.e. VM bit 17 in EFLAGS has not been set by the boot loader
; These assumptions are consistent with the multiboot requirements
; But we additionally assume
; - Paging is enabled which breaks the multiboot compliance
; - We also assume that the boot loader has found a reasonable place for the stack pointer (at least 3 MB away from the start of the kernel)

; Kstart has a simple function :
; - Call the Kmain function which is the entry point to the kernel
; - Kmain will only return when nothing can wake up the system anymore (interrupts are disabled) and the system can peacefully go to sleep forever

; We have decided that the start of the kernel (in virtual memory) is 0xC0100000
; However, we are not going to put an ORG here unlike what we did with the boot loader
; This is because we are going to compile this (and other assembly files) to object files, and link them with object files produced from our C code to produce a binary
; The linker will take care of assigning the correct memory values
; We will provide the linker with a script (link.ld) to tell it exactly where we want it to put various parts of our code
; So, we specifiy here a 'section' for the code instead. This is called 'text' section and that's where all the instructions go
; The 'data' section is where initialized data resides
; The uninitialized data goes into the 'bss' section
; And the read-only data goes in the 'rodata' section 

section .text

global Kstart
Kstart:

	extern Kmain
	call Kmain

	hlt
