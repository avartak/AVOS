; The following are the descriptors for the 32-bit and 16-bit protected mode segments

SEG32_CODE        equ 0x08
SEG32_DATA        equ 0x10
SEG16_CODE        equ 0x18
SEG16_DATA        equ 0x20

section .text

BITS 32

; The 40-byte "BIOS Registry" structure
BIOS_Regs16:
    .eax    dd   0
    .ebx    dd   0
    .ecx    dd   0
    .edx    dd   0
    .esi    dd   0
    .edi    dd   0
    .ebp    dd   0
    .esp    dd   0
    .ds     dw   0
    .es     dw   0
    .ss     dw   0
    .flags  dw   0

BIOS_Int_ID dd 0


BIOS_IDT_Desc:
    dw 0x400 - 1
    dd 0


Save_IDT_Desc:
    dw 0x400 - 1
    dd 0

global BIOS_ClearRegistry
; Parameters (in order that is passed when calling the function in C):
; - Pointer to the BIOS registry structure that needs to be cleared
BIOS_ClearRegistry:
	push  ebp
	mov   ebp, esp

	push  eax
	push  edi
	push  ecx

	mov   edi, [ebp+8]
	mov   eax, 0
	mov   ecx, 10
	rep   stosd

	pop   ecx
	pop   edi
	pop   eax

	mov   esp, ebp
	pop   ebp
	ret

global BIOS_Interrupt
; Parameters (in order that is passed when calling the function in C):
; - Interrupt ID as a 32-bit unsigned integer
; - Pointer to the BIOS registry structure
BIOS_Interrupt:

	push  ebp
	mov   ebp, esp

	; We start by saving the general registers and the flags register to memory, in the BIOS_Regs32 structure defined at the end

	pushfd
	pushad

	; Clear interrupts

	cli

	; We then save the IDT descriptor

	sidt  [Save_IDT_Desc]

	; We then copy the interrupt vector index to the EDX register

	mov   eax, [ebp+8]
	mov   [BIOS_Int_ID], eax

	; We store the real mode register values (from the structure pointed to by EAX) into the BIOS_Regs16 structure

	mov   esi, [ebp+12]
	mov   edi, BIOS_Regs16
	mov   ecx, 10
	rep   movsd
	
	; Switch to 16-bit protected mode

	jmp   SEG16_CODE:BIOS_Interrupt.PMode16

BITS 16

	; Switch the segment registers to 16-bit protected mode (1 MB address space)

	.PMode16:
	mov   ax, SEG16_DATA
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax

	; Switch to real mode by disabling the PE bit in the CR0 register

	mov   eax, cr0
	and   al , 0xFE
	mov   cr0, eax

	; Switch the code segment register to real mode

	jmp   0:BIOS_Interrupt.RMode

	; Switch the segment registers to real mode

	.RMode:
	mov   ax, 0
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax

	; Store the designated values in DS and ES (taken from BIOS_Regs16)

	mov   ax, [fs:BIOS_Regs16.ds]
	mov   ds, ax
	mov   ax, [fs:BIOS_Regs16.es]
	mov   es, ax

	; Store the designated values in the general purpose registers
	; We do not change the stack pointer ; it could be done but we do not need to do it in the context of our boot loader set up

	mov   eax, [fs:BIOS_Regs16.eax]
	mov   ebx, [fs:BIOS_Regs16.ebx]
	mov   ecx, [fs:BIOS_Regs16.ecx]
	mov   edx, [fs:BIOS_Regs16.edx]
	mov   esi, [fs:BIOS_Regs16.esi]
	mov   edi, [fs:BIOS_Regs16.edi]
	mov   ebp, [fs:BIOS_Regs16.ebp]

	; Save the stack pointer in case the BIOS routine trashes it

	mov   [fs:BIOS_Regs16.esp], esp

	; Load the real mode IDT so that we can call the BIOS interrupts
	; In general, we should save the 32-bit IDT descriptor so that we can use it after switching back to protected mode 
	; But we don't have it set up in the boot loader anyways

	lidt  [fs:BIOS_IDT_Desc]

	; Trigger the appropriate BIOS interrupt
	; It is at this stage that we will change the DS register to the designated value

	cmp   WORD [fs:BIOS_Int_ID], 0x10
	je    BIOS_Interrupt.Int0x10
	cmp   WORD [fs:BIOS_Int_ID], 0x12
	je    BIOS_Interrupt.Int0x12
	cmp   WORD [fs:BIOS_Int_ID], 0x13
	je    BIOS_Interrupt.Int0x13
	cmp   WORD [fs:BIOS_Int_ID], 0x15
	je    BIOS_Interrupt.Int0x15
	cmp   WORD [fs:BIOS_Int_ID], 0x16
	je    BIOS_Interrupt.Int0x16

	jmp   BIOS_Interrupt.SwitchToPMode32

	.Int0x10:
	clc
	int   0x10
	jmp   BIOS_Interrupt.SwitchToPMode32

	.Int0x12:
	clc
	int   0x12
	jmp   BIOS_Interrupt.SwitchToPMode32

	.Int0x13:
	clc
    int   0x13
    jmp   BIOS_Interrupt.SwitchToPMode32

	.Int0x15:
	clc
    int   0x15
    jmp   BIOS_Interrupt.SwitchToPMode32

	.Int0x16:
	clc
    int   0x16
    jmp   BIOS_Interrupt.SwitchToPMode32


	; We are done with the BIOS routine; now we have to switch back to the protected mode
	; First we restore the stack pointer and store the register values (these represent the state immediately after the BIOS call) in the BIOS_Regs16 structure

	.SwitchToPMode32:
    mov   esp, [fs:BIOS_Regs16.esp]
	mov   [fs:BIOS_Regs16.eax], eax	
	mov   [fs:BIOS_Regs16.ebx], ebx	
	mov   [fs:BIOS_Regs16.ecx], ecx	
	mov   [fs:BIOS_Regs16.edx], edx	
	mov   [fs:BIOS_Regs16.esi], esi	
	mov   [fs:BIOS_Regs16.edi], edi	
	mov   [fs:BIOS_Regs16.ebp], ebp	

	; Also save the flags register

	pushf
	pop   ax
	mov   [fs:BIOS_Regs16.flags], ax

	; Save the segment registers

	mov   ax, ds
	mov   [fs:BIOS_Regs16.ds], ax	
	mov   ax, es
	mov   [fs:BIOS_Regs16.es], ax	

	; Switch to 32-bit protected mode

    mov   eax, cr0
    or    al , 1
    mov   cr0, eax

    jmp   SEG32_CODE:BIOS_Interrupt.PMode32

BITS 32

	.PMode32:

	; Switch data segments to the 32-bit protected mode and restore the general purpose registers to their values at the time this function was called

	mov   ax, SEG32_DATA
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax
	popad
	popfd

	; Save the real mode register values in the structure whose pointer that was passed to this function as an argument

	push  esi
	push  edi
	push  ecx

	mov   esi, BIOS_Regs16
	mov   edi, [ebp+12]
	mov   ecx, 10
	rep   movsd
	
	pop   ecx
	pop   edi
	pop   esi

	; Restore the IDT that we started out with 

	lidt  [Save_IDT_Desc]

	; Return 

	mov   esp, ebp
	pop   ebp
	ret

