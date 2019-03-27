; This is the code of the 2nd stage of the boot loader
; We are still in REAL mode
; This code will reside at memory location 0x8000
; It will copy the kernel at 1 MB
; This requires jumping from protected mode to unreal mode (full 4 GB address space but 16-bit environment/architecture, and hence, access to BIOS)
; At some later stage, a dedicated 32-bit code will be set up to do the readout instead of falling back on BIOS (if at all this is deemed to be necessary/useful)
; Lastly, we switch back from unreal mode to protected mode, and launch the kernel

; First let us include some definitions of constants (the constants themselves are described in comments)

START_BOOT2             equ 0x8000                                      ; This is where we load the 2nd stage of the boot loader
START_KERNEL            equ 0x100000                                    ; Kernel is loaded at physical memory location of 1 MB
START_SCRATCH           equ 0x7000                                      ; Starting point of the scratch area 
START_GDT               equ 0x7E00                                      ; Memory location of the GDT

SIZE_KERNEL             equ 0x100000/0x200                              ; Assumed size of the kernel binary in terms of number of sectors

SEG32_CODE              equ 0x08                                        ; 32-bit code segment
SEG32_DATA              equ 0x10                                        ; 32-bit data segment

FLOPPY_ID               equ 0                                           ; Floppy ID used by the BIOS
HDD_ID                  equ 0x80                                        ; Floppy ID used by the BIOS

START_KERNEL_DISK       equ 0x40                                        ; Starting sector of the kernel

MULTIBOOT2_MAGIC        equ 0x36d76289                                  ; Need to provide this as input to the kernel to convey that it has been loaded by a multiboot-2 compliant boot loader (which this is not)
MULTIBOOT2_HEADER_SIZE  equ 0x1000                                      ; Multiboot2 header size, more specifically amount of bytes set aside for the header (it's actual size may be less)
MULTIBOOT2_INFO_ADDRESS equ 0x10000                                     ; Physical memory location of the multiboot information structures (MBI)
MULTIBOOT2_INFO_SEGMENT equ 0x0800
MULTIBOOT2_INFO_OFFSET  equ 0x8000                                      ; Location of the start of the boot information structures 


; Starting point of the kernel loader

ORG START_BOOT2

; We are still in 16-bit real mode

BITS 16


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BootStage2:

	; To enable the protected mode we will :
	; - Clear all interrupts
	; - Switch on the A20 line 
	; - Store the memory map (in multiboot2 compatible format) to be used by the kernel during initialization
	; - Create a GDT and load it
	; - Enter protected mode
	; - Enter unreal mode
	; - Copy the kernel from disk memory
	; - Enter back into 32-bit protected mode
	; - Launch the kernel
	
	cli
	
	call SwitchOnA20
	test al, al
	jz   HaltSystem
	
	push MULTIBOOT2_INFO_OFFSET
	push MULTIBOOT2_INFO_SEGMENT
	call StoreMemoryMap
	test al, al
	jz   HaltSystem
	
	push START_GDT
	call LoadGDT
	
	mov  eax, cr0                                       
	or    al, 1
	mov  cr0, eax
	
	mov  ax, SEG32_DATA                                 
	mov  ds, ax                                         
	mov  es, ax                                         
	
	mov  eax, cr0	
	and   al, 0xFE
	mov  cr0, eax
	
	xor  ax, ax
	mov  ds, ax
	mov  es, ax
	
	push HDD_ID
	push START_SCRATCH
	push DWORD START_KERNEL
	push DWORD SIZE_KERNEL
	push DWORD START_KERNEL_DISK
	call ReadFromDisk
	test al, al
	jz   HaltSystem
	
	mov  eax, cr0
	or    al, 1
	mov  cr0, eax
	
	jmp  SEG32_CODE:InProtectedMode
	
; Halt the system in case of trouble
HaltSystem:
	cli
	hlt
	jmp  HaltSystem

; Code to enable the A20 line
%include "x86/boot/src/a20.asm"

; Code to store the memory map
%include "x86/boot/src/memorymap.asm"

; Code to create the GDT
%include "x86/boot/src/gdt.asm"

; Code to read from disk
%include "x86/boot/src/diskread.asm"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	
; We are now firmly 32-bit protected mode territory

BITS 32

InProtectedMode:

	mov  ax, SEG32_DATA
	mov  ds, ax
	mov  es, ax
	mov  fs, ax
	mov  gs, ax
	mov  ss, ax
	
	mov  eax, MULTIBOOT2_MAGIC
	mov  ebx, MULTIBOOT2_INFO_ADDRESS
	
	jmp  START_KERNEL+MULTIBOOT2_HEADER_SIZE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


