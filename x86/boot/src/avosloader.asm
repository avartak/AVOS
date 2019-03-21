; This is the code of the 2nd stage of the boot loader
; We are still in REAL mode
; This code will reside at memory location 0x8000
; It will copy the kernel at 1 MB
; This requires jumping from protected mode to unreal mode (full 4 GB address space but 16-bit environment/architecture, and hence, access to BIOS)
; At some later stage, a dedicated 32-bit code will be set up to do the readout instead of falling back on BIOS (if at all this is deemed to be necessary/useful)
; Lastly, we switch back from unreal mode to protected mode, and launch the kernel

; First let us include some definitions of constants (the constants themselves are described in comments)

%include "x86/boot/src/defs.asm"

; Starting point of the kernel loader

ORG START_BOOT2

; We are still in 16-bit real mode

BITS 16


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
AVOS:

	; To enable the protected mode we will :
	; - Clear all interrupts
	; - Switch on the A20 line 
	; - Store the multiboot information to be used by the kernel during initialization
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
	
	push MULTIBOOT_INFO_OFFSET
	push MULTIBOOT_INFO_SEGMENT
	call StoreMultibootInfo
	test al, al
	jz   HaltSystem
	
	push START_GDT
	call LoadGDT
	
	push ds
	push es
	
	mov  eax, cr0                                       
	or   eax, 1
	mov  cr0, eax
	
	mov  ax, SEG32_DATA                                 
	mov  ds, ax                                         
	mov  es, ax                                         
	
	mov  eax, cr0	
	and  al , 0xFE
	mov  cr0, eax
	
	pop  es
	pop  ds
	
	push HDD_ID
	push START_SCRATCH
	push DWORD START_KERNEL
	push DWORD SIZE_KERNEL/SECTOR_SIZE
	push DWORD START_KERNEL_DISK
	call ReadFromDisk
	test al, al
	jz   HaltSystem
	
	mov eax, cr0
	or  eax, 1
	mov cr0, eax
	
	jmp SEG32_CODE:InProtectedMode
	
; Halt the system in case of trouble
HaltSystem:
	cli
	hlt
	jmp HaltSystem

; Code to enable the A20 line
%include "x86/boot/src/a20.asm"

; Code to store the memory map
%include "x86/boot/src/multibootinfo.asm"

; Code to create the GDT
%include "x86/boot/src/gdt.asm"

; Code to read from disk
%include "x86/boot/src/diskread.asm"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	
; We are now firmly 32-bit protected mode territory

BITS 32

InProtectedMode:

	mov ax, SEG32_DATA
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	mov eax, MULTIBOOT_MAGIC
	mov ebx, MULTIBOOT_INFO_ADDRESS
	
	jmp START_KERNEL+MULTIBOOT_HEADER_SIZE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


