section .text

global X86_IDT_Load
X86_IDT_Load:
    mov   edx, [esp+4]
    lidt [edx]

    ret

