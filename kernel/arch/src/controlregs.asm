section .text

global X86_CR0_Write
X86_CR0_Write:
	mov eax, [esp+4]
    mov cr0, eax
    ret

global X86_CR3_Write
X86_CR3_Write:
    mov eax, [esp+4]
    mov cr3, eax
    ret

global X86_CR4_Write
X86_CR4_Write:
    mov eax, [esp+4]
    mov cr4, eax
    ret

global X86_CR0_Read
X86_CR0_Read:
    mov eax, cr0
    ret

global X86_CR2_Read
X86_CR2_Read:
    mov eax, cr2
    ret

global X86_CR3_Read
X86_CR3_Read:
    mov eax, cr3
    ret

global X86_CR4_Read
X86_CR4_Read:
    mov eax, cr4
    ret

