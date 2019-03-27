BITS 32

section .text

%include "kernel/src/multiboot.asm"

times 0x1000-($-$$) db 0

MULTIBOOT2_MAGIC            equ 0x36d76289
KERNEL_HIGHER_HALF_OFFSET   equ 0xC0000000

KERN_MEM_MIN         equ 0x0100000
KERN_MEM_MAX         equ 0x0400000
DISP_MEM_MIN         equ 0x1000000
DISP_MEM_MAX         equ 0x1400000

STACK_TOP            equ 0x00400000	

global AVOS
AVOS:

	cli

	mov  esp, AVOS-KERNEL_HIGHER_HALF_OFFSET

	cmp  eax, MULTIBOOT2_MAGIC
	jne  HaltSystem

	extern MBI_address
	mov [MBI_address-KERNEL_HIGHER_HALF_OFFSET], ebx
	add [MBI_address-KERNEL_HIGHER_HALF_OFFSET], DWORD 0xC0000000

	extern Physical_Memory_CheckRange
	push ebx
	push KERN_MEM_MAX
	push KERN_MEM_MIN
	call Physical_Memory_CheckRange
	add  esp, 8
	test al, al
	jz   HaltSystem

	push DISP_MEM_MAX
	push DISP_MEM_MIN
	call Physical_Memory_CheckRange
	add  esp, 8
	test al, al
	jz   HaltSystem

	pop  ebx
	mov  esp, STACK_TOP

	extern InitializeSystem
	jmp  InitializeSystem

	HaltSystem:
	cli
	hlt
	jmp  HaltSystem
