%include "kernel/core/setup/include/setup.inc"

%define PHYSADDR(x) (x-KERNEL_HIGHER_HALF_OFFSET)

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

	mov   esp, PHYSADDR(Start+KERNEL_STACK_SIZE)

	cmp   eax, 0x36d76289
	jne   Halt

	add   ebx, KERNEL_HIGHER_HALF_OFFSET
	mov   [PHYSADDR(BootInfo_Ptr)], ebx

	call  Initialize_Paging
	call  Initialize_HigherHalfSwitch
	call  Initialize_CRT
	jmp   Initialize_System

	Halt:
    hlt
    jmp  Halt

	times KERNEL_STACK_SIZE-($-$$) db 0

section .data

	global BootInfo_Ptr
	BootInfo_Ptr:
	dd 0
