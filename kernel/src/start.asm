section .text

global Kstart
Kstart:

	cli
	mov esp, 0xC0300000
 
	extern Kmain
	call Kmain

	hlt

