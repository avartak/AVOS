section .text

global Kstart
Kstart:

	cli
	mov esp, 0xC0400000
 
	extern Kmain
	call Kmain
 
	hlt

