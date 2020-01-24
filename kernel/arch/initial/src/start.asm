%include "kernel/core/setup/include/setup.inc"

extern Initialize_Paging
extern Initialize_HigherHalfSwitch
extern Initialize_CRT
extern Initialize_GDT
extern Initialize_IDT
extern Initialize_APICs
extern Initialize_PreInitDevices
extern Console_PrintWelcome

section .text

BITS 32

global Start
Start:

	cli
	cld 

	mov   esp, Start+KERNEL_STACK_PTR_OFFSET-KERNEL_HIGHER_HALF_OFFSET

	cmp   eax, 0x36d76289
	jne   HaltSystem

	add   ebx, KERNEL_HIGHER_HALF_OFFSET
	mov   [BootInfo_Ptr-KERNEL_HIGHER_HALF_OFFSET], ebx

	call  Initialize_Paging
	call  Initialize_HigherHalfSwitch
	call  Initialize_GDT
	call  Initialize_IDT
	call  Initialize_CRT

	call  Initialize_APICs
	call  Initialize_PreInitDevices

	call  Console_PrintWelcome

	HaltSystem:
    hlt
    jmp  HaltSystem

	times KERNEL_STACK_PTR_OFFSET-($-$$) db 0

section .data

	global BootInfo_Ptr
	BootInfo_Ptr:
	dd 0
