BITS 32

section .start

%include "kernel/src/multiboot.asm"

times 0x1000-($-$$) db 0

Stack_Temp:

section .text

MULTIBOOT2_BOOTLOADER_MAGIC equ 0x36d76289
HIGHER_HALF_OFFSET          equ 0xC0000000
STACK_TOP                   equ 0x00400000	

global Kstart
Kstart:

	cli

	mov esp, Stack_Temp

	cmp eax, MULTIBOOT2_BOOTLOADER_MAGIC
	jne End

	push ebx
	extern Initial_Checks
	call   Initial_Checks
	cmp eax, 0
	je  End
	pop ebx

	mov  esp, STACK_TOP

	add  ebx, HIGHER_HALF_OFFSET
	push ebx

	extern Paging_Initialize
	call   Paging_Initialize

	mov eax, High_Memory
	jmp eax
	High_Memory:
	add esp, HIGHER_HALF_OFFSET

	extern Paging_directory
	mov   [Paging_directory], DWORD 0

	extern Kinit
	call   Kinit

	sti

    extern Kmain
    jmp    Kmain

	End:
	cli
	hlt

