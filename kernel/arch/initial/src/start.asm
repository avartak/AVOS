%include "kernel/core/setup/include/setup.inc"

extern Initialize_HigherHalf
extern Initialize_Tables
extern Initialize_CRT
extern Initialize_ThisProcessor
extern Initialize_Stack
extern Initialize_Memory
extern Initialize_System
extern GetToWork

section .text

BITS 32

global Start
Start:

	cli
	cld 

	mov   esp, KERNEL_AP_BOOT_START_ADDR+KERNEL_AP_BOOT_START_SIZE

	cmp   eax, 0x36d76289
	jne   Halt

	add   ebx, KERNEL_HIGHER_HALF_OFFSET
	mov   [PHYSADDR(BootInfo_Ptr)], ebx

	call  Initialize_HigherHalf
	call  Initialize_CRT
	call  Initialize_Tables
	call  Initialize_ThisProcessor
	call  Initialize_Stack
	call  Initialize_System
	call  GetToWork

	Halt:
    hlt
    jmp  Halt

section .data

	global BootInfo_Ptr
	BootInfo_Ptr:
	dd 0
