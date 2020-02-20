section .text

global X86_Outb
X86_Outb:
	mov dx, [esp+4]
	mov al, [esp+8]
	out dx, al
	ret

global X86_Outw
X86_Outw:
    mov dx, [esp+4]
    mov ax, [esp+8]
    out dx, ax
	ret

global X86_Outd
X86_Outd:
    mov  dx, [esp+4]
    mov eax, [esp+8]
    out  dx, eax
	ret

global X86_Inb
X86_Inb:
    mov  dx, [esp+4]
    in   al, dx
	ret

global X86_Inw
X86_Inw:
    mov  dx, [esp+4]
    in   ax, dx
    ret  

global X86_Ind
X86_Ind:
    mov  dx, [esp+4]
    in  eax, dx
    ret  


