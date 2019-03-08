GDT_KERN_CODE_SEG  equ 0x08
GDT_KERN_DATA_SEG  equ 0x10

section .text

global GDT_Load
GDT_Load:
	mov eax, [esp+4]
	lgdt [eax]	

	ret

global TSS_LoadTaskRegister
TSS_LoadTaskRegister:
	mov eax, [esp+4]
	ltr ax

	ret

global GDT_LoadKernelSegments
GDT_LoadKernelSegments:
	jmp GDT_KERN_CODE_SEG:.reloadCS
	.reloadCS:
	mov  ax, GDT_KERN_DATA_SEG
	mov  ds, ax
	mov  es, ax
	mov  fs, ax
	mov  gs, ax
	mov  ss, ax

	ret

