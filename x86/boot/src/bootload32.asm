; This is the code of the 2nd stage of the boot loader
; We are still in REAL mode
; This code is loaded at memory location 0x7E00 right after the 512 B of the boot sector code
; It will switch to PROTECED mode and then copy the kernel at 1 MB

; First let us include some definitions of constants

STACK_TOP               equ 0x7000                                      ; Top of the stack - it can extend down till 0x500 without running into the BIOS data area (0x400-0x500)

KERNEL_START            equ 0x100000                                    ; Kernel binary code is loaded at physical memory location of 1 MB
KERNEL_IMAGE_START      equ 0x100000                                    ; Kernel ELF executable is loaded at physical memory location of 1 MB
KERNEL_IMAGE_SIZE       equ 0x100000                                    ; Size of the kernel image in bytes
KERNEL_DISK_START       equ 0x1000                                      ; Starting sector of the kernel

SEG32_DATA              equ 0x10                                        ; 32-bit data segment

SCREEN_TEXT_BUFFER      equ 0xB8000                                     ; Video buffer for the 80x25 VBE text mode

; These are the externally defined functions and variables we need

extern IO_PrintBanner
extern IO_MakeCursorInvisible
extern DiskIO_ReadFromDisk
extern Multiboot_CreateEmptyMBI
extern Multiboot_SaveMemoryMaps
extern Multiboot_LoadKernel
extern Multiboot_SaveInfo

; Starting point of the kernel loader --> in the .boot section, following immediately after the 512 B of the boot sector code

section .text

; We are now in 32-bit protected mode

BITS 32


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

global Bootload32
Bootload32:

	; Switch the data segment registers to protected mode data selectors

	mov  ax, SEG32_DATA
	mov  ds, ax
	mov  es, ax
	mov  fs, ax
	mov  gs, ax
	mov  ss, ax

	mov  esp, STACK_TOP

    ; Store information needed to load the kernel

    mov  [Kernel_Info.boot_drive_ID], dl

    mov  [Kernel_Info.boot_partition], esi

    mov  eax, KERNEL_DISK_START
    mov  [Kernel_Info.disk_start], eax

    mov  eax, KERNEL_IMAGE_START
    mov  [Kernel_Info.image_start], eax

    mov  eax, KERNEL_START
    mov  [Kernel_Info.start], eax

    mov  eax, KERNEL_IMAGE_SIZE
    mov  [Kernel_Info.image_size], eax


	; Print the AVOS boot loader banner

	call IO_PrintBanner
	call IO_MakeCursorInvisible

	; Create an empty multiboot information (MBI) record

	push Multiboot_Information_start
	call Multiboot_CreateEmptyMBI
	add  esp, 0x4

	; Save memory information in MBI

	push Multiboot_Information_start
	call Multiboot_SaveMemoryMaps
	add  esp, 0x4
	test al, al
	mov  esi, ErrStr_Memory
	jz   HaltSystem

	; Load kernel from disk

	push  Multiboot_Information_start
	push  Kernel_Info 
	call  Multiboot_LoadKernel
	add   esp, 0x8
	test  al, al
	mov   esi, ErrStr_LoadKernel
	jz    HaltSystem

	; Store multiboot information

	push Kernel_Info
	push Multiboot_Information_start
	call Multiboot_SaveInfo
	add  esp, 0x8
	test al, al
	mov  esi, ErrStr_MBI 
	jz   HaltSystem
	
	; Store the pointer to the boot information table in EBX

	mov  ebx, Multiboot_Information_start

	; Store the Multiboot2 boot loader magic value in EAX

	mov  eax, 0x36d76289

	; Jump to the kernel

	jmp  DWORD [Kernel_Info.entry]

    HaltSystem:
    mov   edi, SCREEN_TEXT_BUFFER+80*23*2
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


section .data

; Error strings in case the boot loader runs into trouble

Messages: 
ErrStr_A20          db 'A20 line could not be enabled', 0
ErrStr_Memory       db 'Unable to get memory information', 0
ErrStr_LoadKernel   db 'Unable to load kernel', 0
ErrStr_MBI          db 'Unable to save multiboot information', 0

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

; Kernel loading information

align 8

global Kernel_Info
Kernel_Info:
	.boot_drive_ID     resd 1
	.boot_partition    resd 1
	.image_size        resd 1
	.image_start       resd 1
	.disk_start        resd 1
	.start             resd 1
	.multiboot_header  resd 1
	.entry             resd 1
	.size              resd 1
	.reserved          resd 1

; Multiboot information (MBI) table

global Multiboot_Information_start
Multiboot_Information_start:

