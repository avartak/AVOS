%include "kernel/core/setup/include/setup.inc"

section .text

global Initialize_HigherHalf
Initialize_HigherHalf:
	extern Kernel_pagedirectory
	mov  eax, PHYSADDR(Kernel_pagedirectory)
	mov  cr3, eax

	mov  eax, cr4
	or   eax, 0x10
	mov  cr4, eax

	mov  eax, cr0
	or   eax, 0x80010000
	mov  cr0, eax

	mov  eax, High_Memory
	jmp  eax
	High_Memory:
	add  esp , DWORD KERNEL_HIGHER_HALF_OFFSET
	add [esp], DWORD KERNEL_HIGHER_HALF_OFFSET
	ret

global Initialize_Stack
Initialize_Stack:
    extern Page_Acquire
	push KERNEL_STACK_SIZE >> 12
	call Page_Acquire
	add  esp, 4

	mov  edx, [esp]
    mov  esp,  eax
	add  esp, KERNEL_STACK_SIZE-KERNEL_STRUCT_CPU_SIZE-KERNEL_STRUCT_STATE_SIZE
    push edx
    ret


