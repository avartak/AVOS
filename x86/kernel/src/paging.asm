KERNEL_HIGHER_HALF_OFFSET equ 0xC0000000

extern Paging_directory

section .text

BITS 32

global Paging_Enable
Paging_Enable:
	mov  edx, cr0
	or   edx, 0x80000000
	mov  cr0, edx
	ret

global Paging_LoadDirectory
Paging_LoadDirectory:
	mov  edx, [esp+4]
	mov  cr3, edx
	ret

global Paging_SwitchToHigherHalf
Paging_SwitchToHigherHalf:
    mov  eax, High_Memory
    jmp  eax
    High_Memory:
	add [esp], DWORD KERNEL_HIGHER_HALF_OFFSET
    add  esp , DWORD KERNEL_HIGHER_HALF_OFFSET
    add  ebp , DWORD KERNEL_HIGHER_HALF_OFFSET
    mov  [Paging_directory], DWORD 0
    ret
