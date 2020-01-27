%include "kernel/core/setup/include/setup.inc"

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
	.end:
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
	.end:
	ret

global X86_GetStackBase
X86_GetStackBase:
	mov  eax, esp
	and  eax, KERNEL_STACK_SIZE
	add  eax, KERNEL_STACK_SIZE

global X86_SwitchToHigherHalf
X86_SwitchToHigherHalf:
	mov  eax, High_Memory
	jmp  eax
	High_Memory:
	add  esp   , DWORD KERNEL_HIGHER_HALF_OFFSET
	add [esp]  , DWORD KERNEL_HIGHER_HALF_OFFSET
	ret

