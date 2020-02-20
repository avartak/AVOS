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
	extern Initialize_KernelStackOffset

	push (KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS)
	call Page_Acquire
	add  esp, 4

	mov  edx, [esp]
    mov  esp,  eax
	add  esp, KERNEL_STACK_SIZE
	call Initialize_KernelStackOffset
	sub  esp, eax
	and  esp, ~(KERNEL_STACK_ALIGNMENT-1)
    push edx
    ret


