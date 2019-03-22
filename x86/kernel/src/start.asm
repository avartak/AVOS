BITS 32

section .text

%include "kernel/src/multiboot.asm"

times 0x1000-($-$$) db 0

Stack_Temp:

MULTIBOOT2_BOOTLOADER_MAGIC equ 0x36d76289
HIGHER_HALF_OFFSET          equ 0xC0000000
STACK_TOP                   equ 0x00400000	

KERN_MEM_MIN                equ 0x0100000
KERN_MEM_MAX                equ 0x0400000
DISP_MEM_MIN                equ 0x1000000
DISP_MEM_MAX                equ 0x1400000

global Kstart
Kstart:

	extern MBI_address
	extern Physical_Memory_CheckRange
	extern Kinit
	extern PrintNum

	cli

	cmp  eax, MULTIBOOT2_BOOTLOADER_MAGIC
	jne  HaltSystem

	mov  esp, Stack_Temp-HIGHER_HALF_OFFSET

	push ebx
	push KERN_MEM_MAX
	push KERN_MEM_MIN
	call Physical_Memory_CheckRange
	add  esp, 8
	test al, al
	jz   HaltSystem

	push ebx
	push DISP_MEM_MAX
	push DISP_MEM_MIN
	call Physical_Memory_CheckRange
	add  esp, 8
	test al, al
	jz   HaltSystem

	pop  ebx
	add  ebx, HIGHER_HALF_OFFSET
	mov  [MBI_address-HIGHER_HALF_OFFSET], ebx

	mov  esp, STACK_TOP

	call Kinit

	HaltSystem:
	cli
	hlt
	jmp  HaltSystem

