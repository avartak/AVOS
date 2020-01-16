GDT_KERN_CODE_SEG  equ 0x08
GDT_KERN_DATA_SEG  equ 0x10

section .text

global X86_GDT_Load
X86_GDT_Load:
    mov   edx, [esp+4]
    lgdt [edx]

    ret

global X86_GDT_LoadTaskRegister
X86_GDT_LoadTaskRegister:
	mov  edx, [esp+4]
	ltr  dx

	ret

global X86_GDT_LoadKernelSegments
X86_GDT_LoadKernelSegments:
    jmp  GDT_KERN_CODE_SEG:X86_GDT_LoadKernelSegments.reloadCS
    .reloadCS:
    mov  dx, GDT_KERN_DATA_SEG
    mov  ds, dx
    mov  es, dx
    mov  fs, dx
    mov  gs, dx
    mov  ss, dx

    ret

