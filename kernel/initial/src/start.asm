KERNEL_STACK_PTR_OFFSET   equ 0x6000
KERNEL_HIGHER_HALF_OFFSET equ 0xC0000000

extern Initialize_Paging
extern Initialize_SwitchToHigherHalf
extern Initialize_CRT
extern Initialize_GDT
extern Initialize_IDT
extern Welcome

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
	call  Initialize_SwitchToHigherHalf
	call  Initialize_GDT
	call  Initialize_IDT
	call  Initialize_CRT
	call  Welcome

	HaltSystem:
    cli
    hlt
    jmp  HaltSystem

	times KERNEL_STACK_PTR_OFFSET-($-$$) db 0

section .data

	global BootInfo_Ptr
	BootInfo_Ptr:
	dd 0
