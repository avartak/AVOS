BITS 32

section .text

MULTIBOOT2_MAGIC   equ 0x36d76289
KERNEL_START       equ 0x100000
STACK_SIZE         equ 0x1000

%include "kernel/src/multiboot.asm"

times STACK_SIZE-($-$$) db 0

global Start
Start:

	cli

	mov  esp, KERNEL_START+STACK_SIZE

	cmp  eax, MULTIBOOT2_MAGIC
	jne  HaltSystem

	extern LaunchAVOS
	push ebx
	call LaunchAVOS

	HaltSystem:
	cli
	hlt
	jmp  HaltSystem
