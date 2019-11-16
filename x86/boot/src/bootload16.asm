; This is the code of the 16-bit (REAL mode) part of the bootloader
; We are still in REAL mode
; It will switch to PROTECTED mode and transfer control to the 32-bit boot loader that will then copy the kernel at 1 MB

; Some notes about the mapping of the first MB of the physical memory in the PC architecture, and how we use this memory 

; 0x000000 - 0x000400 : Interrupt Vector Table  
; 0x000400 - 0x000500 : BIOS data area          
; 0x000500 - 0x007C00 : Free area
; 0x000600 - 0x000800 : Relocated MBR (It's stack top is at 0x0600)
; 0x000800 - 0x001000 : Stack of the relocated MBR
; 0x001000 - 0x007000 : Stack of the boot loader
; 0x007C00 - 0x007E00 : VBR (It's stack top is at 0x7C00)
; 0x007E00 - 0x09FC00 : Free area
; 0x007E00 - 0x008000 : 16-bit part of the boot loader code (It's stack top is at 0x7C00)
; 0x008000 - 0x00FFFF : 32-bit part of the boot loader code (It's stack top is at 0x7C00)    
; 0x09FC00 - 0x0A0000 : Extended BIOS data area 
; 0x0A0000 - 0x0C0000 : Video memory            
; 0x0C0000 - 0x100000 : BIOS            

; First let us include some definitions of constants

START_ADDRESS           equ 0x7E00                                      ; The Volume Boot Record (VBR) puts this code right after it in memory (so 0x7C00 + 0x200)
STACK_TOP               equ 0x7C00                                      ; Top of the stack
SCREEN_TEXT_BUFFER      equ 0xB800                                      ; Video buffer for the 80x25 VBE text mode (for displaying error messages)
SEG32_CODE              equ 0x08                                        ; 32-bit kernel code segment

; Starting point of the bootloader in memory --> follows immediately after the 512 bytes of the VBR

ORG START_ADDRESS

; We are still in 16-bit real mode

BITS 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Bootload16:

    ; We don't want any interrupts right now.

    cli

    ; Clear the direction flag so that the string operations (that we will use later) proceed from low to high memory addresses

    cld

    ; We first set up a usable stack at 0x7000

    xor   ax, ax
    mov   ss, ax
    mov   sp, STACK_TOP

    ; The VBR stores the boot drive ID in DL and the active partition table entry in the relocated MBR in DS:SI when control is transferred to the boot loader. 
    ; Save these registers on the stack

    push  dx
    push  ds
    push  si

	; Set all the segment registers to the base address we want (0x0000)
	
	xor   ax, ax
	mov   ds, ax	
	mov   es, ax	
	jmp   0x0000:Start

	; To enable the protected mode we will :
	; - Switch on the A20 line 
	; - Create a GDT and load it
	; - Set the first bit in CR0 to 1
	
	Start:

	; Set video to 80x25 text mode

	mov   ax, 0x0003
	int   0x10

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

