%include "kernel/arch/processor/include/gdt.inc"

section .text

global GDT_Load
GDT_Load:
    mov   edx, [esp+4]
    lgdt [edx]

    ret

global GDT_LoadKernelSegments
GDT_LoadKernelSegments:
    jmp  GDT_SEG_KERN_CODE:GDT_LoadKernelSegments.reloadCS
    .reloadCS:
    mov  dx, GDT_SEG_KERN_DATA
    mov  ds, dx
    mov  es, dx
    mov  fs, dx
    mov  gs, dx
    mov  ss, dx

    ret

global TSS_LoadTaskRegister
TSS_LoadTaskRegister:
    mov  edx, [esp+4]
    ltr  dx

    ret

