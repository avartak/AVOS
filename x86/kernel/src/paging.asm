KERNEL_HIGHER_HALF_OFFSET equ 0xC0000000

section .text

extern Paging_directory

global Paging_Enable
Paging_Enable:
	mov  eax, cr0
	or   eax, 0x80000000
	mov  cr0, eax
	ret

global Paging_LoadDirectory
Paging_LoadDirectory:
	mov  eax, [esp+4]
	mov  cr3, eax
	ret

global Paging_SwitchToHigherHalf
Paging_SwitchToHigherHalf:
    mov  eax, High_Memory
    jmp  eax
    High_Memory:
	add [esp], DWORD KERNEL_HIGHER_HALF_OFFSET
    add  esp , DWORD KERNEL_HIGHER_HALF_OFFSET
    mov  [Paging_directory], DWORD 0
    ret

