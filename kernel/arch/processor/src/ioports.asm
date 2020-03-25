section .text

global Outb
Outb:
	mov dx, [esp+4]
	mov al, [esp+8]
	out dx, al
	ret

global Outw
Outw:
    mov dx, [esp+4]
    mov ax, [esp+8]
    out dx, ax
	ret

global Outd
Outd:
    mov  dx, [esp+4]
    mov eax, [esp+8]
    out  dx, eax
	ret

global Inb
Inb:
    mov  dx, [esp+4]
    in   al, dx
	ret

global Inw
Inw:
    mov  dx, [esp+4]
    in   ax, dx
    ret  

global Ind
Ind:
    mov  dx, [esp+4]
    in  eax, dx
    ret  


