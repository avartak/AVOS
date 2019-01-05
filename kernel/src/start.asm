section .bss
align 16
stack_bottom:
resb 16384
stack_top:
 

section .text
global start
start:

	mov esp, stack_top
 
	extern kmain
	call kmain
 
	cli
.hang:	hlt
	jmp .hang
.end:

