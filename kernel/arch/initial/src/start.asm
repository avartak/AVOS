%include "kernel/core/setup/include/setup.inc"

extern Initialize_Paging
extern Initialize_HigherHalfSwitch
extern Initialize_CRT
extern Initialize_System

section .text

BITS 32

global Start
Start:

	cli
	cld 

	mov   esp, Start+KERNEL_STACK_SIZE-KERNEL_HIGHER_HALF_OFFSET

	cmp   eax, 0x36d76289
	jne   HaltSystem

	add   ebx, KERNEL_HIGHER_HALF_OFFSET
	mov   [BootInfo_Ptr-KERNEL_HIGHER_HALF_OFFSET], ebx

	call  Initialize_Paging
	call  Initialize_HigherHalfSwitch
	call  Initialize_CRT
	call  Initialize_System

	HaltSystem:
    hlt
    jmp  HaltSystem

	times KERNEL_STACK_SIZE-($-$$) db 0

section .data

	global BootInfo_Ptr
	BootInfo_Ptr:
	dd 0
