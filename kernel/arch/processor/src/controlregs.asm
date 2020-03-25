section .text

global CR0_Write
CR0_Write:
	mov eax, [esp+4]
    mov cr0, eax
    ret

global CR3_Write
CR3_Write:
    mov eax, [esp+4]
    mov cr3, eax
    ret

global CR4_Write
CR4_Write:
    mov eax, [esp+4]
    mov cr4, eax
    ret

global CR0_Read
CR0_Read:
    mov eax, cr0
    ret

global CR2_Read
CR2_Read:
    mov eax, cr2
    ret

global CR3_Read
CR3_Read:
    mov eax, cr3
    ret

global CR4_Read
CR4_Read:
    mov eax, cr4
    ret

