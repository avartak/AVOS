; First let us include some definitions of constants

%include "kernel/core/setup/include/setup.inc"
%include "kernel/arch/i386/include/gdt.inc"

; Macro to manipulate label addresses

%define BOOTADDR(x) (x-StartAP+KERNEL_AP_BOOT_START_ADDR)
%define PHYSADDR(x) (x-KERNEL_HIGHER_HALF_OFFSET)

; External functions we will be calling

extern Initialize_AP_Paging
extern Initialize_AP
extern X86_SwitchToHigherHalf

; Code section

section .text

; We are still in 16-bit real mode

BITS 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

global StartAP
StartAP:

	; First disable interrupts

	cli

	; Load a valid GDT (See the GDT description at the end)

	lgdt  [BOOTADDR(GDT.Pointer)]
	
	; Enter protected mode by setting bit 0 of the CR0 register, and making a long-jump using a 32-bit code segment (which switches CS to protected mode)

	mov   eax, cr0                                       
	or    al , 1
	mov   cr0, eax

	jmp   GDT.Code32:BOOTADDR(ProtectedMode)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; We are now in 32-bit protected mode

section .text

BITS 32

	ProtectedMode:
	jmp   GDT.Code32:PHYSADDR(StartAPReloc)

	StartAPReloc:
	
	; Switch all registers to protected mode
	
	mov   ax, GDT.Data32
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax
	mov   esp, KERNEL_AP_BOOT_START_ADDR+KERNEL_AP_BOOT_START_SIZE

	call  Initialize_AP_Paging
	call  X86_SwitchToHigherHalf
	call  Initialize_AP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Description of the GDT is given at the end 

align 8

GDT:
	.Null   : equ $-GDT 
	dq 0
	.Code32 : equ $-GDT 
	dq 0x00CF9A000000FFFF
	.Data32 : equ $-GDT 
	dq 0x00CF92000000FFFF
	.Code16 : equ $-GDT 
	dq 0x000F9A000000FFFF
	.Data16 : equ $-GDT 
	dq 0x000F92000000FFFF
	.Pointer:
	dw $-GDT-1
	dd GDT

	times KERNEL_AP_BOOT_START_SIZE-($-$$) db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
