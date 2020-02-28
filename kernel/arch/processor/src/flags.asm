section .text

global ReadFlags
ReadFlags:
	pushfd 
    pop eax
    ret

global ReadEFlags
ReadEFlags:
	pushfd 
    pop eax
    ret

