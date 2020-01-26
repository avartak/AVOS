%include "kernel/core/setup/include/setup.inc"

%define PHYSADDR(x) (x-KERNEL_HIGHER_HALF_OFFSET)

extern Welcome
extern Initialize_BSP_Paging
extern Initialize_CRT
extern Initialize_BSP
extern Initialize_System
extern X86_SwitchToHigherHalf

section .text

BITS 32

global Start
Start:

	cli
	cld 

	mov   esp, PHYSADDR(Start+KERNEL_STACK_SIZE)

	cmp   eax, 0x36d76289
	jne   Halt

	add   ebx, KERNEL_HIGHER_HALF_OFFSET
	mov   [PHYSADDR(BootInfo_Ptr)], ebx

	call  Initialize_BSP_Paging
	call  X86_SwitchToHigherHalf
	call  Initialize_CRT

	call  Welcome
	call  Initialize_BSP
	call  Initialize_System

	Halt:
    hlt
    jmp  Halt

	times KERNEL_STACK_SIZE-($-$$) db 0

section .data

	global BootInfo_Ptr
	BootInfo_Ptr:
	dd 0
