%include "kernel/initial/include/setup.inc"

extern Kernel_pagedirectory

section .text

BITS 32

global Initialize_SwitchToHigherHalf
Initialize_SwitchToHigherHalf:
    mov  eax, High_Memory
    jmp  eax
    High_Memory:
	add [esp], DWORD KERNEL_HIGHER_HALF_OFFSET
    add  esp , DWORD KERNEL_HIGHER_HALF_OFFSET
    add  ebp , DWORD KERNEL_HIGHER_HALF_OFFSET
    mov  [Kernel_pagedirectory], DWORD 0
    ret
