; This is the code of the 32-bit (REAL mode) part of the bootloader
; We are now in PROTECTED mode
; This code is loaded at memory location 0x8000 right after the 512 B of VBR (at 0x7C00) followed by 512 B of the real mode part of the bootloader (at 0x7E00)
; It prepares the environment for the OS kernel and then loads it from disk into memory location of 1 MB. 
; The kernel is located on the disk partition at an offset of 1 MB. 
; We try to make the bootloader and the OS kernel as Multiboot2 compliant as possible. 
; Multiboot2 specs : https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html

; First let us include some definitions of constants

STACK_TOP               equ 0x7C00                                      ; Top of the stack
KERNEL_START            equ 0x100000                                    ; Kernel binary code is loaded at physical memory location of 1 MB
SEG32_DATA              equ 0x10                                        ; 32-bit data segment
SCREEN_TEXT_BUFFER      equ 0xB8000                                     ; Video buffer for the 80x25 VBE text mode

; These are the externally defined functions and variables we need

extern Multiboot_Boot

; Starting point of the 32-bit bootloader code. See linkboot.ld for details on how the code is linked into the bootloader binary

section .text

; We are now in 32-bit protected mode

BITS 32


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

global AVBL
AVBL:

	; First lets disable all interrupts (the real mode code should have done this already before switching to protected mode, but lets do it anyways)
	
	cli
	
	; Switch the data segment registers to protected mode data selectors
	
	mov   ax, SEG32_DATA
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax
	
	mov   esp, STACK_TOP
	
	; Store information passed on by the VBR
	
	mov   [Kernel_Info.boot_drive_ID], dl
	mov   [Kernel_Info.pnpbios_check_ptr], edi
	mov   [Kernel_Info.boot_partition], esi
	mov   [Kernel_Info.part_info_ptr], ebp
	mov   [Kernel_Info.blocklist_ptr], ebx
	
	; Boot OS 
	
	push  Kernel_Info 
	push  Multiboot_MBI
	call  Multiboot_Boot
	add   esp, 0x8
	test  al, al
	jz    HaltSystem
	
	; Store the pointer to the boot information table in EBX
	
	mov   ebx, Multiboot_MBI
	
	; Store the Multiboot2 boot loader magic value in EAX
	
	mov   eax, 0x36d76289
	
	; Jump to the kernel
	
	jmp   DWORD [Kernel_Info.entry]
	
	HaltSystem:
	cli
	hlt
	jmp   HaltSystem

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


section .data

; Kernel loading information

global Kernel_Info
Kernel_Info:
	.boot_drive_ID     dd 0
	.pnpbios_check_ptr dd 0
	.boot_partition    dd 0
	.part_info_ptr     dd 0
	.blocklist_ptr     dd 0
	.start             dd KERNEL_START
	.size              dd 0
	.bss_size          dd 0
	.multiboot_header  dd 0
	.entry             dd 0
	.file_addr         dd 0
	.file_size         dd 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


section .bss

align 8

; Multiboot information (MBI) table

global Multiboot_MBI
Multiboot_MBI:
