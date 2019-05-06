; This is the code of the 2nd stage of the boot loader
; We are still in REAL mode
; This code is loaded at memory location 0x7E00 right after the 512 B of the boot sector code
; It will switch to PROTECED mode and transfer control to the 32-bit boot loader that will then copy the kernel at 1 MB

; First let us include some definitions of constants

STACK_TOP               equ 0x7000                                      ; Top of the stack - it can extend down till 0x500 without running into the BIOS data area (0x400-0x500)
SEG32_CODE              equ 0x08                                        ; 32-bit code segment

SCREEN_TEXT_BUFFER      equ 0xB800                                      ; Video buffer for the 80x25 VBE text mode

; Starting point of the kernel loader --> in the .boot section, following immediately after the 512 B of the boot sector code

ORG 0x7E00

; We are still in 16-bit real mode

BITS 16


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Bootload16:

	; To enable the protected mode we will :
	; - Clear all interrupts
	; - Switch on the A20 line 
	; - Create a GDT and load it
	; - Set the first bit in CR0 to 1
	
	cli

	jmp   0x0000:Start

	Start:
	xor   ax, ax
	mov   ds, ax	
	mov   es, ax	
	mov   ss, ax	
	mov   sp, STACK_TOP

	; Save the boot drive ID and active partition entry location on the stack

	push  dx
	push  bx
	push  si

	; Enable the A20 line

	call  A20_Enable
	test  al, al
    mov   si, ErrStr_A20
	jz    HaltSystem

	; Save the boot drive ID and the active partition entry location in DL, ESI respectively

	mov   sp, STACK_TOP
	sub   sp, 0x6
	mov   eax, 0
	mov   esi, 0
	pop   ax
	pop   si
	pop   dx
	shl   esi, 4
	add   esi, eax

	; Load a valid GDT

	lgdt [GDT_Desc]
	
	; Enter protected mode

	mov   eax, cr0                                       
	or    al , 1
	mov   cr0, eax

	; Switch CS to protected mode descriptor with a far jump	

	jmp   SEG32_CODE:ProtectedMode
	
	; Halt the system in case of trouble

	HaltSystem:
    mov   ax, SCREEN_TEXT_BUFFER
    mov   es, ax
    mov   di, 80*23*2
    .printchar:
        lodsb
        test  al, al
        jz    .printdone
        mov   ah, 0x04
        stosw
        jmp   .printchar
    .printdone:
	cli
	hlt
	jmp  .printdone



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Enable the A20 line

%include "x86/boot/src/a20.asm"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ErrStr_A20 db 'A20 line could not be enabled', 0

GDT: 
	dq 0
	dq 0x00CF9A000000FFFF
	dq 0x00CF92000000FFFF
	dq 0x000F9A000000FFFF
	dq 0x000F92000000FFFF

GDT_Desc:
	dw GDT_Desc - GDT - 1
	dd GDT

times 512-($-$$) db 0
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; We are now in 32-bit protected mode

BITS 32

ProtectedMode:

