; The following care the descriptors for the 32-bit and 16-bit protected mode segments

SEG32_CODE           equ 0x08
SEG32_DATA           equ 0x10
SEG16_CODE           equ 0x18
SEG16_DATA           equ 0x20

REAL_MODE_STACK_SIZE equ 0x400
INTERRUPTS_ENABLED   equ 0x200

section .text

BITS 32

align 0x10

BIOS_Start:

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

; Interrupt number to be invoked
BIOS_Int_ID db 0

; Place to store the protected mode stack pointer (we will use a dedicated 1 KB stack for the real mode)
BIOS_PM_ESP dd   0 

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
; - Interrupt ID
; - Pointer to the BIOS registry structure
BIOS_Interrupt:

	push  ebp
	mov   ebp, esp

	; We start by saving the general registers and the flags register on the stack (pushing the flags register twice is deliberate - used at the end to set the interrupt flag)

	pushfd
	pushfd
	pushad

	; Clear interrupts

	cli

	; Save the interrupt vector index

	mov   al, [ebp+8]
	mov   [BIOS_Int_ID], al

	; Save the real mode register values to be used when calling the BIOS routine

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

	; Set the base address of FS to BIOS_Start

	mov   eax, BIOS_Start
	shr   eax, 4
	mov   fs, ax

	; Lets set up a real mode stack

	mov   [fs:BIOS_PM_ESP-BIOS_Start], esp
	mov   eax, BIOS_Stack_Segment
	shr   eax, 4
	mov   ss, ax
	mov   esp, BIOS_Stack_Pointer-BIOS_Stack_Segment
	
	; Lets set data segments (DS,ES,GS) all with base address 0

	xor   ax, ax
	mov   ds, ax
	mov   es, ax
	mov   gs, ax

	; Switch the code segment register to real mode with a far return, setting the base address of CS to BIOS_Start

	push  fs
	push  BIOS_Interrupt.RMode-BIOS_Start
	retf

	.RMode:

	; Save the real mode stack pointer in case the BIOS routine trashes it

	mov   [fs:BIOS_Regs16.esp-BIOS_Start], esp

	; Prepare the stack for a call to the interrupt routine

    mov   al, BYTE [fs:BIOS_Int_ID-BIOS_Start]
    mov   bl, 4
    mul   bl
	mov   si, ax

    pushf
    push  cs
    push  WORD BIOS_Interrupt.SwitchToPMode32-BIOS_Start
    push  DWORD [si]

	; Store the designated values in DS and ES (taken from BIOS_Regs16)

	mov   ax, [fs:BIOS_Regs16.ds-BIOS_Start]
	mov   ds, ax
	mov   ax, [fs:BIOS_Regs16.es-BIOS_Start]
	mov   es, ax

	; Store the designated values in the general purpose registers

	mov   eax, [fs:BIOS_Regs16.eax-BIOS_Start]
	mov   ebx, [fs:BIOS_Regs16.ebx-BIOS_Start]
	mov   ecx, [fs:BIOS_Regs16.ecx-BIOS_Start]
	mov   edx, [fs:BIOS_Regs16.edx-BIOS_Start]
	mov   esi, [fs:BIOS_Regs16.esi-BIOS_Start]
	mov   edi, [fs:BIOS_Regs16.edi-BIOS_Start]
	mov   ebp, [fs:BIOS_Regs16.ebp-BIOS_Start]

	; Trigger the appropriate BIOS interrupt

	clc 
    retf

	; We are done with the BIOS routine; now we have to switch back to the protected mode
	; First we restore the stack pointer and store the register values (these represent the state immediately after the BIOS call) in the BIOS_Regs16 structure

	.SwitchToPMode32:
    mov   esp, [fs:BIOS_Regs16.esp-BIOS_Start]
	mov   [fs:BIOS_Regs16.eax-BIOS_Start], eax	
	mov   [fs:BIOS_Regs16.ebx-BIOS_Start], ebx	
	mov   [fs:BIOS_Regs16.ecx-BIOS_Start], ecx	
	mov   [fs:BIOS_Regs16.edx-BIOS_Start], edx	
	mov   [fs:BIOS_Regs16.esi-BIOS_Start], esi	
	mov   [fs:BIOS_Regs16.edi-BIOS_Start], edi	
	mov   [fs:BIOS_Regs16.ebp-BIOS_Start], ebp	

	; Also save the flags register

	pushf
	pop   ax
	mov   [fs:BIOS_Regs16.flags-BIOS_Start], ax

	; Save the segment registers

	mov   ax, ds
	mov   [fs:BIOS_Regs16.ds-BIOS_Start], ax	
	mov   ax, es
	mov   [fs:BIOS_Regs16.es-BIOS_Start], ax	

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

	mov   esp, [BIOS_PM_ESP]
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

	; Check if interrupts were enabled when BIOS_Interrupt was called

	pop   eax
	test  eax, INTERRUPTS_ENABLED
	jz    .return
	sti

	; Return 

	.return:
	mov   esp, ebp
	pop   ebp
	ret



align 0x10

BIOS_Stack_Segment:
times REAL_MODE_STACK_SIZE db   0
BIOS_Stack_Pointer:

