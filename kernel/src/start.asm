; This is the start of our kernel
; It's a simple function that will :
; - Clear all the interrupts (they have been disabled anyways, but still)
; - Set the stack pointer to 0xC0400000 (so 3 MB away from the start of the kernel code)
; - Call the Kmain function which is the entry point to the C code of the kernel
; - Enter into a sleep leep
;
; The sleep loop is basically a halt instruction followed by a jump to the halt instruction. Why not just a halt ?
; Lets assume we got to the point of the sleep loop. At that point the system is in an idle state
; The only thing that will wake it up is an interrupt -- for example, lets say you press a key
; Now, the interrupt will do what it needs to do and then return to the next instruction after the point where it injected itself
; If the system is sitting at its very last instruction, there is no next instruction and it will go crazy 
; The sleep loop makes sure that does not happen 


; We know that the start of the kernel is 0xC0100000
; However, we are not going to put an ORG here unlike what we did with the boot loader
; This is because we are going to compile this (and other assembly files) to object files, and link them with object files produced from our C code to produce a binary
; The linker will take care of assigning the correct memory values
; We will provide the linker with a script (link.ld) to tell it exactly where we want it to put various parts of our code
; So, we specifiy here a code section instead. The text section is where all the instructions go
; The data section is where initialized data resides
; The uninitialized data goes into the bss sections
; And the read-only data goes in the rodata section 

; We could make our kernel multiboot compliant so that it can be loaded with an external boot loader like GRUB
; That requires adding a multiboot header at the beginning of the kernel 
; It hasn't been done so far 

section .text

global Kstart
Kstart:

	cli 

	mov esp, 0xC0400000
 
	extern Kmain
	call Kmain

	.sleeploop:
		hlt
		jmp .sleeploop

