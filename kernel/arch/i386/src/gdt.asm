%include "kernel/arch/i386/include/gdt.inc"

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
    jmp  X86_GDT_SEG_KERN_CODE:X86_GDT_LoadKernelSegments.reloadCS
    .reloadCS:
    mov  dx, X86_GDT_SEG_KERN_DATA
    mov  ds, dx
    mov  es, dx
    mov  fs, dx
    mov  gs, dx
    mov  ss, dx

    ret

