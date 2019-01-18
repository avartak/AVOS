.text

global SysCall
SysCall:

	push eax	
	mov  eax, [esp+8]

	int 0x80

	pop eax

	ret
