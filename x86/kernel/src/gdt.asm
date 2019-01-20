section .text

global LoadKernelSegments
LoadKernelSegments:
	jmp 0x08:.reloadCS

	.reloadCS:
	mov eax, 0x10
	mov  ds, eax
	mov  es, eax
	mov  fs, eax
	mov  gs, eax
	mov  ss, eax

	ret

global LoadUserSegments
LoadUserSegments:
    jmp 0x18:.reloadCS

    .reloadCS:
    mov eax, 0x20
    mov  ds, eax
    mov  es, eax
    mov  fs, eax
    mov  gs, eax
    mov  ss, eax

    ret

