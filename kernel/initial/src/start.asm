KERNEL_STACK_PTR_OFFSET   equ 0x6000
KERNEL_HIGHER_HALF_OFFSET equ 0xC0000000

extern Paging_Initialize
extern Paging_SwitchToHigherHalf
extern CRT_Initialize
extern GDT_Initialize
extern IDT_Initialize
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


	call  Paging_Initialize
	call  Paging_SwitchToHigherHalf
	call  CRT_Initialize
	call  GDT_Initialize
	call  IDT_Initialize
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
