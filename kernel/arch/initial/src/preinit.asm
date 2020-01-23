%include "kernel/core/setup/include/setup.inc"

extern Kernel_pagedirectory

section .text

BITS 32

global Initialize_HigherHalfSwitch
Initialize_HigherHalfSwitch:
	mov  eax, High_Memory
	jmp  eax
	High_Memory:
	add [esp]  , DWORD KERNEL_HIGHER_HALF_OFFSET
	add  esp   , DWORD KERNEL_HIGHER_HALF_OFFSET
	mov  [Kernel_pagedirectory], DWORD 0
	ret
