BITS 32

section .text

KERNEL_START            equ 0x100000
KERNEL_STACK_PTR_OFFSET equ 0x6000

global Start
Start:

	cli

	mov  esp, KERNEL_START+KERNEL_STACK_PTR_OFFSET

	extern AVOS_Launch
	push ebx
	push eax
	call AVOS_Launch

	%include "kernel/src/multiboot.asm"

	times KERNEL_STACK_PTR_OFFSET-($-$$) db 0
