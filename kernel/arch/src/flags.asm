section .text

global X86_ReadFlags
X86_ReadFlags:
	pushfd 
    pop eax
    ret

global X86_ReadEFlags
X86_ReadEFlags:
	pushfd 
    pop eax
    ret

