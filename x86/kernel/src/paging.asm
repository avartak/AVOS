HIGHER_HALF_OFFSET          equ 0xC0000000

section .text

extern Paging_directory

global Paging_SwitchToHigherHalf
Paging_SwitchToHigherHalf:

    push ebp
    mov  ebp, esp

    mov eax, High_Memory
    jmp eax
    High_Memory:
    mov [Paging_directory], DWORD 0

    mov eax, [ebp+4]
    add eax, HIGHER_HALF_OFFSET
    mov [ebp+4], eax
    add ebp, HIGHER_HALF_OFFSET

    mov esp, ebp
    pop ebp
    ret
