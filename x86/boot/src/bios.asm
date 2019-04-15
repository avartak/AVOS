
SEG32_CODE   equ 0x08
SEG32_DATA   equ 0x10
SEG16_CODE   equ 0x18
SEG16_DATA   equ 0x20


BITS 32

global BIOS_Interrupt
BIOS_Interrupt:

	mov   [BIOS_Regs32.eax], eax
	mov   [BIOS_Regs32.ebx], ebx
	mov   [BIOS_Regs32.ecx], ecx
	mov   [BIOS_Regs32.edx], edx
	mov   [BIOS_Regs32.esi], esi
	mov   [BIOS_Regs32.edi], edi
	mov   [BIOS_Regs32.ebp], ebp
	mov   [BIOS_Regs32.esp], esp

	mov   ebx, [esp+4]
	mov   [BIOS_Int_ID], ebx

	mov   eax, [esp+8]
	mov   ebx, [eax+0x00]
	mov   [BIOS_Regs16.eax], ebx
	mov   ebx, [eax+0x04]
	mov   [BIOS_Regs16.ebx], ebx
	mov   ebx, [eax+0x08]
	mov   [BIOS_Regs16.ecx], ebx
	mov   ebx, [eax+0x0C]
	mov   [BIOS_Regs16.edx], ebx
	mov   ebx, [eax+0x10]
	mov   [BIOS_Regs16.esi], ebx
	mov   ebx, [eax+0x14]
	mov   [BIOS_Regs16.edi], ebx
	mov   ebx, [eax+0x18]
	mov   [BIOS_Regs16.ebp], ebx
	mov   ebx, [eax+0x1C]
	mov   [BIOS_Regs16.esp], ebx

	mov   bx , [eax+0x20]
	mov   [BIOS_Regs16.ds], bx
	mov   bx , [eax+0x22]
	mov   [BIOS_Regs16.es], bx
	mov   bx , [eax+0x24]
	mov   [BIOS_Regs16.ss], bx

	jmp   SEG16_CODE:BIOS_Interrupt.PMode16

BITS 16

	.PMode16:
	mov   ax, SEG16_DATA
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax

	mov   eax, cr0
	and   al , 0xFE
	mov   cr0, eax

	jmp   0:BIOS_Interrupt.RMode

	.RMode:
	mov   ax, 0
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax

	mov   ax, [BIOS_Regs16.es]
	mov   es, ax

	mov   eax, [BIOS_Regs16.eax]
	mov   ebx, [BIOS_Regs16.ebx]
	mov   ecx, [BIOS_Regs16.ecx]
	mov   edx, [BIOS_Regs16.edx]
	mov   esi, [BIOS_Regs16.esi]
	mov   edi, [BIOS_Regs16.edi]
	mov   ebp, [BIOS_Regs16.ebp]

	push  ds

	lidt  [IDT_Desc]

	cmp   WORD [BIOS_Int_ID], 0x10
	je    BIOS_Interrupt.Int0x10
	cmp   WORD [BIOS_Int_ID], 0x13
	je    BIOS_Interrupt.Int0x13
	cmp   WORD [BIOS_Int_ID], 0x15
	je    BIOS_Interrupt.Int0x15

	jmp   BIOS_Interrupt.SwitchToPMode32

	.Int0x10:
	push  ax
	mov   ax, [BIOS_Regs16.ds]
	mov   ds, ax
	pop   ax
	int   0x10
	jmp   BIOS_Interrupt.SwitchToPMode32

	.Int0x13:
	push  ax
	mov   ax, [BIOS_Regs16.ds]
	mov   ds, ax
	pop   ax
	int   0x13
	jmp   BIOS_Interrupt.SwitchToPMode32

	.Int0x15:
	push  ax
	mov   ax, [BIOS_Regs16.ds]
	mov   ds, ax
	pop   ax
	int   0x15
	jmp   BIOS_Interrupt.SwitchToPMode32

	.SwitchToPMode32:
	pop   ds
	mov   [BIOS_Regs16.eax], eax	
	mov   [BIOS_Regs16.ebx], ebx	
	mov   [BIOS_Regs16.ecx], ecx	
	mov   [BIOS_Regs16.edx], edx	
	mov   [BIOS_Regs16.esi], esi	
	mov   [BIOS_Regs16.edi], edi	
	mov   [BIOS_Regs16.ebp], ebp	
	mov   [BIOS_Regs16.esp], esp	

	pushfd
	pop   eax
	mov   [BIOS_Regs16.flags], ax

	mov   ax, ds
	mov   [BIOS_Regs16.ds], ax	
	mov   ax, es
	mov   [BIOS_Regs16.es], ax	

    mov   eax, cr0
    or    al , 1
    mov   cr0, eax

    jmp   SEG32_CODE:BIOS_Interrupt.PMode32

BITS 32

	.PMode32:

	mov   ax, SEG32_DATA
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax
	mov   esp, [BIOS_Regs32.esp]	

    mov   eax, [esp+8]
    mov   ebx, [BIOS_Regs16.eax]
    mov   [eax+0x00], ebx
    mov   ebx, [BIOS_Regs16.ebx]
    mov   [eax+0x04], ebx
    mov   ebx, [BIOS_Regs16.ecx]
    mov   [eax+0x08], ebx
    mov   ebx, [BIOS_Regs16.edx]
    mov   [eax+0x0C], ebx
    mov   ebx, [BIOS_Regs16.esi]
    mov   [eax+0x10], ebx
    mov   ebx, [BIOS_Regs16.edi]
    mov   [eax+0x14], ebx
    mov   ebx, [BIOS_Regs16.ebp]
    mov   [eax+0x18], ebx
    mov   ebx, [BIOS_Regs16.esp]
    mov   [eax+0x1C], ebx

    mov   bx , [BIOS_Regs16.ds]
    mov   [eax+0x20], bx
    mov   bx , [BIOS_Regs16.es]
    mov   [eax+0x22], bx
    mov   bx , [BIOS_Regs16.ss]
    mov   [eax+0x24], bx
    mov   bx , [BIOS_Regs16.flags]
    mov   [eax+0x26], bx

	mov   eax, [BIOS_Regs32.eax]	
	mov   ebx, [BIOS_Regs32.ebx]	
	mov   ecx, [BIOS_Regs32.ecx]	
	mov   edx, [BIOS_Regs32.edx]	
	mov   esi, [BIOS_Regs32.esi]	
	mov   edi, [BIOS_Regs32.edi]	
	mov   ebp, [BIOS_Regs32.ebp]	
	mov   esp, [BIOS_Regs32.esp]	

	ret



section .data

BIOS_Regs32:
	.eax    dd   0
	.ebx    dd   0
	.ecx    dd   0
	.edx    dd   0
	.esi    dd   0
	.edi    dd   0
	.ebp    dd   0
	.esp    dd   0


BIOS_Regs16:
	.eax    dd   0
	.ebx    dd   0
	.ecx    dd   0
	.edx    dd   0
	.esi    dd   0
	.edi    dd   0
	.ebp    dd   0
	.esp    dd   0
	.flags  dw   0
	.ds     dw   0
	.es     dw   0
	.ss     dw   0

BIOS_Int_ID dd 0


IDT_Desc:
    dw 0x400 - 1
    dd 0
    dw 0


