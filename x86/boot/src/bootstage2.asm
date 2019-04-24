; This is the code of the 2nd stage of the boot loader
; We are still in REAL mode
; This code is loaded at memory location 0x7E00 right after the 512 B of the boot sector code
; It will switch to PROTECED mode and then copy the kernel at 1 MB

; First let us include some definitions of constants

STACK_TOP               equ 0x7000                                      ; Top of the stack - it can extend down till 0x500 without running into the BIOS data area (0x400-0x500)

KERNEL_START            equ 0x100000                                    ; Kernel is loaded at physical memory location of 1 MB
KERNEL_SIZE             equ 0x100000/0x200                              ; Assumed size of the kernel binary in terms of number of sectors
KERNEL_DISK_START       equ 0x40                                        ; Starting sector of the kernel

SEG32_CODE              equ 0x08                                        ; 32-bit code segment
SEG32_DATA              equ 0x10                                        ; 32-bit data segment

MEMORY_ADDRESS_CHECK    equ 0x00C00000                                  ; Check if the system has usable RAM from 1 MB to this value 

; These are the externally defined functions and variables we need

extern A20_Enable
extern DiskIO_ReadFromDisk
extern BootInfo_Store
extern RAM_IsMemoryPresent

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
    mov  si, ErrStr_A20
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
    mov   ax, 0xB800
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
	jmp  HaltSystem16

	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	; We are now in 32-bit protected mode

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
	add  esp, 0x10
	test al, al
	mov  esi, ErrStr_DiskIO
	jz   HaltSystem32
	
	; Store boot information

	push Boot_Tables
	push BootInfo_Table
	call BootInfo_Store
	add  esp, 0x8

	; Check if the system has 1-12 MB of usable address space 

	push MEMORY_ADDRESS_CHECK 
	push KERNEL_START
	call RAM_IsMemoryPresent
	add  esp, 0x8
	test al, al
	mov  esi, ErrStr_Memory
	jz   HaltSystem32

	; Store the pointer to the boot information table

	mov  ebx, BootInfo_Table
	
	; Jump to the kernel

	jmp  KERNEL_START

    HaltSystem32:
    mov   edi, 0xB8000+80*23*2
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
    jmp  HaltSystem32

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


section .data

; Error strings in case the boot loader runs into trouble

Messages: 
ErrStr_A20     db 'A20 line could not be enabled', 0
ErrStr_DiskIO  db 'Unable to read kernel image from disk', 0
ErrStr_Memory  db 'Insufficient memory available', 0

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


section .bss

; Drive ID needed to read from disk using BIOS INT 0x13 routine

Boot_DriveID    resd 1

; Boot information table

global BootInfo_Table
BootInfo_Table  resq 0x20

; This is where the boot information tables start

Boot_Tables:

