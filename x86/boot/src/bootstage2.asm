; This is the code of the 2nd stage of the boot loader
; We are still in REAL mode
; This code is loaded at memory location 0x7E00 right after the 512 B of the boot sector code
; It will switch to PROTECED mode and then copy the kernel at 1 MB

; First let us include some definitions of constants (the constants themselves are described in comments)

STACK_TOP               equ 0x7000                                      ; Top of the stack - it can extend down till 0x500 without running into the BIOS data area (0x400-0x500)

KERNEL_START            equ 0x100000                                    ; Kernel is loaded at physical memory location of 1 MB
KERNEL_SIZE             equ 0x100000/0x200                              ; Assumed size of the kernel binary in terms of number of sectors
KERNEL_DISK_START       equ 0x40                                        ; Starting sector of the kernel

SEG32_CODE              equ 0x08                                        ; 32-bit code segment
SEG32_DATA              equ 0x10                                        ; 32-bit data segment

; These are the externally defined functions and variables we need

extern A20_Enable
extern DiskIO_ReadFromDisk
extern BootInfo_Store

; Starting point of the kernel loader --> in the .boot section, following immediately after the 512 B of the boot sector code

section .boot

; We are still in 16-bit real mode

BITS 16


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

BootStage2:

	; To enable the protected mode we will :
	; - Clear all interrupts
	; - Switch on the A20 line 
	; - Create a GDT and load it
	; - Set the first bit in CR0 to 1
	
	cli

	jmp 0x0000:Start

	Start:
	xor  ax, ax
	mov  ds, ax	
	mov  es, ax	
	mov  ss, ax	
	mov  sp, STACK_TOP

	; Store the ID of the boot drive -- will need it to copy the kernel

	mov  [Boot_DriveID], dl

	; Enable the A20 line

	call A20_Enable
	test al, al
	jz   HaltSystem16

	; Load a valid GDT

	lgdt [GDT_Desc]
	
	; Enter protected mode

	mov  eax, cr0                                       
	or   al , 1
	mov  cr0, eax

	; Switch CS to protected mode descriptor with a far jump	

	jmp  SEG32_CODE:ProtectedMode
	
	; Halt the system in case of trouble

	HaltSystem16:
	cli
	hlt
	jmp  HaltSystem16

	
	; We are now firmly 32-bit protected mode territory

BITS 32

	ProtectedMode:

	; Switch the remaining segment registers to protected mode data segment selectors

	mov  ax, SEG32_DATA
	mov  ds, ax
	mov  es, ax
	mov  fs, ax
	mov  gs, ax
	mov  ss, ax

	; Copy kernel from disk to high memory

	push KERNEL_SIZE
	push KERNEL_DISK_START
	push KERNEL_START
	push DWORD [Boot_DriveID]
	call DiskIO_ReadFromDisk
	test al, al
	jz   HaltSystem32
	
	; Store boot information

	call BootInfo_Store
	test al, al
	jz   HaltSystem32

	; Store the pointer to the boot information table

	mov  ebx, BootInfo_Table
	
	; Jump to the kernel

	jmp  KERNEL_START

    HaltSystem32:
    cli
    hlt
    jmp  HaltSystem32

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


section .data

; GDT with 32-bit (0x08, 0x10) and 16-bit (0x18, 0x20) entries

GDT: 
	dq 0
	dq 0x00CF9A000000FFFF
	dq 0x00CF92000000FFFF
	dq 0x000F9A000000FFFF
	dq 0x000F92000000FFFF

GDT_Desc:
	dw GDT_Desc - GDT - 1
	dd GDT
	dw 0


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


section .bss

; Drive ID needed to read from disk using BIOS INT 0x13 routine

Boot_DriveID    resd 1

; Boot information table

global BootInfo_Table
BootInfo_Table  resq 0x20

; This is where the boot information tables start

global Boot_Tables
Boot_Tables:




