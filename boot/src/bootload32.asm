; This is the code of the 32-bit (PROTECTED mode) part of the bootloader
; This code is loaded at memory location 0x8000 right after the 512 B of VBR (at 0x7C00) followed by 512 B of the real mode part of the bootloader (at 0x7E00)
; It prepares the environment for the OS kernel and then loads it from disk into memory location of 1 MB. 
; We try to make the bootloader and the OS kernel as Multiboot2 compliant as possible. 
; Multiboot2 specs : https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html

; These are the externally defined functions and variables we need

extern Multiboot_Boot

; Starting point of the 32-bit bootloader code. See linkboot.ld for details on how the code is linked into the bootloader binary

section .text

; We are now in 32-bit protected mode

BITS 32


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

global AVBL
AVBL:

	; When we get here, all segment registers (including, code, data, stack) are correctly set up, interrupts are disabled, and the stack points to an appropriate location
	; We need to store the information passed on by the VBR into a kernel information structure to be used by the bootloader code (written in C)
	
	mov   [Kernel_Info.boot_drive_ID],   dl
	mov   [Kernel_Info.pnpbios_ptr],    edi
	mov   [Kernel_Info.boot_partition], esi
	mov   [Kernel_Info.part_info_ptr],  ebp
	mov   [Kernel_Info.blocklist_ptr],  ebx
	
	; Boot OS 
	
	push  Kernel_Info 
	push  Multiboot_MBI
	call  Multiboot_Boot
	add   esp, 0x8
	test  al, al
	jz    HaltSystem
	
	; Store the pointer to the boot information table in EBX
	
	mov   ebx, Multiboot_MBI
	
	; Store the Multiboot2 bootloader magic value in EAX
	
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
	.pnpbios_ptr       dd 0
	.boot_partition    dd 0
	.part_info_ptr     dd 0
	.blocklist_ptr     dd 0
	.start             dd 0
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

