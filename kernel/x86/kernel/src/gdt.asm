GDT_KERN_CODE_SEG  equ 0x08
GDT_KERN_DATA_SEG  equ 0x10

section .text

global GDT_Load
GDT_Load:
	mov   edx, [esp+4]
	lgdt [edx]	

	ret

global TSS_LoadTaskRegister
TSS_LoadTaskRegister:
	mov  edx, [esp+4]
	ltr  dx

	ret

global GDT_LoadKernelSegments
GDT_LoadKernelSegments:
	jmp  GDT_KERN_CODE_SEG:GDT_LoadKernelSegments.reloadCS
	.reloadCS:
	mov  dx, GDT_KERN_DATA_SEG
	mov  ds, dx
	mov  es, dx
	mov  fs, dx
	mov  gs, dx
	mov  ss, dx

	ret

