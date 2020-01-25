section .text

global X86_Halt
X86_Halt:
	hlt
	ret

global X86_DisableInterrupts
X86_DisableInterrupts:
	pushfd
	pop  eax
	test eax, 0x200
	mov  eax, 0
	jnz  .end
	mov  al, 0xFF
	.end
	cli
	ret

global X86_EnableInterrupts
X86_EnableInterrupts:
	sti
	ret

global X86_RestoreInterrupts
X86_RestoreInterrupts:
	mov  eax, [esp+4]
	cmp  al, 0xFF
	je   .end
    sti
	.end
    ret
