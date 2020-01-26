; First let us include some definitions of constants

%include "kernel/core/setup/include/setup.inc"
%include "kernel/arch/i386/include/gdt.inc"

; External functions we will be calling

extern Initialize_Paging_ForAPs
extern Initialize_HigherHalfSwitch
extern Initialize_AP

; Code section

section .text

; We are still in 16-bit real mode

BITS 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

global BOOT
BOOT:

	; First disable interrupts

	cli

	; Load a valid GDT (See the GDT description at the end)

	lgdt  [GDT.Pointer]
	
	; Enter protected mode by setting bit 0 of the CR0 register, and making a long-jump using a 32-bit code segment (which switches CS to protected mode)

	mov   eax, cr0                                       
	or    al , 1
	mov   cr0, eax

	jmp   GDT.Code32:ProtectedMode-BOOT+KERNEL_AP_BOOT_START_ADDR

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; We are now in 32-bit protected mode

section .text

BITS 32

	ProtectedMode:
	jmp   GDT.Code32:BOOTReloc-KERNEL_HIGHER_HALF_OFFSET

	BOOTReloc:
	
	; Switch all registers to protected mode
	
	mov   ax, GDT.Data32
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax
	mov   esp, 0x9000

    call Initialize_Paging_ForAPs
    call Initialize_HigherHalfSwitch
    call Initialize_AP

	Halt:
	hlt
	cli
	jmp  Halt-BOOT+KERNEL_AP_BOOT_START_ADDR

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
	.Pointer: equ $-BOOT+KERNEL_AP_BOOT_START_ADDR
	dw $-GDT-1
	dd GDT

	times 0x1000-($-$$) db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

