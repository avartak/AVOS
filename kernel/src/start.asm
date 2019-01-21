; Kstart is the start of our kernel

; We assume the following to be done by the boot loader : 
; - System in protected mode (PE bit : enabled in protected mode is set)
; - The A20 gate is enabled
; - The code segment (CS) is a valid protected mode code segment with an offset of 0 and limit of 0xFFFFFFFF 
; - Segments DS, ES, FS, GS, SS are valid protected segments with an offset of 0 and limit of 0xFFFFFFFF
; - Kernel is placed at 0x100000
; - No assumption about paging being enabled i.e. PG bit (bit 31) in CR0 being set
; - Interrupt flag (IF bit 9) in EFLAGS is cleared i.e. maskable interrupts are disabled
; - Virtual 8086 mode is disabled i.e. VM bit 17 in EFLAGS has not been set by the boot loader
; These assumptions are consistent with the multiboot requirements

; Kstart has a simple function :
; - Clear all the interrupts (they have been disabled anyways, but still)
; - Set the stack pointer to 0x400000 (so 3 MB away from the start of the kernel code)
; - Call the Kmain function which is the entry point to the C code of the kernel
; - Enter into a sleep loop
;
; The sleep loop is basically a halt instruction followed by a jump to the halt instruction. Why not just a halt ?
; Lets assume we got to the point of the sleep loop. At that point the system is in an idle state
; The only thing that will wake it up is an interrupt -- for example, lets say you press a key
; Now, the interrupt will do what it needs to do and then return to the next instruction after the point where it injected itself
; If the system is sitting at its very last instruction, there is no next instruction and it will go crazy 
; The sleep loop makes sure that does not happen 


; We have decided that the start of the kernel is 0xC0100000
; However, we are not going to put an ORG here unlike what we did with the boot loader
; This is because we are going to compile this (and other assembly files) to object files, and link them with object files produced from our C code to produce a binary
; The linker will take care of assigning the correct memory values
; We will provide the linker with a script (link.ld) to tell it exactly where we want it to put various parts of our code
; So, we specifiy here a code section instead. The text section is where all the instructions go
; The data section is where initialized data resides
; The uninitialized data goes into the bss section
; And the read-only data goes in the rodata section 

; We make our kernel multiboot-2 compliant
; It needs an appropriate multiboot header
; This is included from multiboot.asm -- check the file for details on multiboot-2 header specifications

section .multiboot

%include "kernel/src/multiboot.asm"

section .text

global Kstart
Kstart:

	mov esp, 0x400000

	extern Kmain
	call Kmain

	.sleeploop:
		hlt
		jmp .sleeploop

