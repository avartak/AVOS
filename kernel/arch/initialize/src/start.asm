%include "kernel/core/setup/include/setup.inc"

extern Initialize_HigherHalf
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

	cmp   eax, 0x36d76289
	jne   Halt

	add   ebx, KERNEL_HIGHER_HALF_OFFSET
	mov   [PHYSADDR(BootInfo)], ebx

	mov   esp, KERNEL_AP_BOOT_START_ADDR+KERNEL_AP_BOOT_START_SIZE

	call  Initialize_HigherHalf
	call  Initialize_CRT
	call  Initialize_Memory
	call  Initialize_Stack
	call  Initialize_ThisProcessor
	call  Initialize_System

	jmp   GetToWork

	Halt:
    hlt
    jmp  Halt

section .data

	global BootInfo
	BootInfo:
	dd 0
