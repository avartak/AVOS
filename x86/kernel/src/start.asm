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

	extern Physical_Memory_CheckRange
	extern MBI_address
	extern Paging_Initialize
	extern Paging_directory
	extern Kinit
    extern Kmain

	cli

	mov esp, Stack_Temp-HIGHER_HALF_OFFSET

	cmp eax, MULTIBOOT2_BOOTLOADER_MAGIC
	jne End

	push KERN_MEM_MAX
	push KERN_MEM_MIN
	push ebx
	call Physical_Memory_CheckRange
	cmp eax, 0
	je  End
	pop ebx
	add esp, 8

	push DISP_MEM_MAX
	push DISP_MEM_MIN
	push ebx
	call Physical_Memory_CheckRange
	cmp eax, 0
	je  End
	pop ebx

	mov  esp, STACK_TOP

	call Paging_Initialize

	mov eax, High_Memory
	jmp eax
	High_Memory:
	add esp, HIGHER_HALF_OFFSET
	mov [Paging_directory], DWORD 0

	add  ebx, HIGHER_HALF_OFFSET
	mov  [MBI_address], ebx

	call Kinit
	sti
    jmp  Kmain

	End:
	cli
	hlt

