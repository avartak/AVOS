section .text

global IDT_Load
IDT_Load:
    mov   edx, [esp+4]
    lidt [edx]

    ret

