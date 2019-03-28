BITS 32

section .text

MULTIBOOT2_MAGIC   equ 0x36d76289
KERNEL_START       equ 0x00100000
KERNEL_STACK_SIZE  equ 0x00004000

global Start
Start:

	cli

	mov  esp, KERNEL_START+KERNEL_STACK_SIZE

	cmp  eax, MULTIBOOT2_MAGIC
	jne  HaltSystem

	extern LaunchAVOS
	push ebx
	call LaunchAVOS

	HaltSystem:
	cli
	hlt
	jmp  HaltSystem

	%include "kernel/src/multiboot.asm"

	times KERNEL_STACK_SIZE-($-$$) db 0

