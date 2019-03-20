; This is the code of the 2nd stage of the boot loader
; We are still in REAL mode
; This code is expected to reside at memory starting from 0x8000
; The goal is to enable protected mode, copy the 3rd stage of the boot loader to 0xA000, copy the kernel at 1 MB
; Copying the kernel at 1 MB will require jumping from protected mode to the Unreal mode (full 4 GB address space but 16-bit environment/architecture, and hence, access to BIOS)
; At some later stage, a dedicated 32-bit code will be set up to do the readout instead of falling back on BIOS (if at all this is deemed to be necessary/useful)
; Lastly, switch back from Unreal mode to protected mode, and launch the 3rd stage (32-bit) of the boot loader 

; First let us include some definitions of constants (the constants themselves are described in comments)

%include "x86/boot/src/defs.asm"

; Starting point of the kernel loader

ORG START_BOOT2

; We are still in real mode and coding will have to be in assembly (no C for 16-bit code)

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
	; - Check if BIOS extensions exist
	; - Copy the kernel into memory
	; - Enter back into 32-bit protected mode
	; - Launch the kernel

	cli                                                 ; Clear all interrupts so that we won't be disturbed            

	call SwitchOnA20                                    ; Check and enable A20. Code in a20.asm
	test al, al
	jz   HaltSystem

	push MULTIBOOT_INFO_OFFSET
	push MULTIBOOT_INFO_SEGMENT
	call StoreMultibootInfo
	test al, al
	jz   HaltSystem

	push START_GDT
	call LoadGDT

	; This is the code that reads the 3rd (32-bit) stage of the boot loader and the kernel 
	; It reads certain number of sectors from certain locations of the disk
	; We do not have a file system yet, so this is how it will have to be 
	; But when we do get around to having a file system, this is the part of the code should be updated  
	; Basically we will need it to read the kernel image file and put it in memory; for now we simply read certain sectors from disk to memory
	; To read from disk we will use the BIOS routine INT 0x13, AH=0x42
	; This is a BIOS extension and may not be present on old systems. We need to first check if it exists

	push ds
	push es
	
	mov  eax, cr0                                       ; Enter protected mode
	or   eax, 1
	mov  cr0, eax
	
	mov  ax, SEG32_DATA                                 ; We will update the DS and ES segment register to 32-bit mode
	mov  ds, ax                                         ; The MOV will also load the protected mode segment descriptor
	mov  es, ax                                         ; On switching back to real mode the descriptor (and hence the register limit, size, etc.) will stay as is -- this is the unreal mode
	
	mov  eax, cr0	
	and  al , 0xFE                                      ; Switch back to real mode
	mov  cr0, eax
	
	pop  es
	pop  ds

	push HDD_ID
	call BIOS_CheckForExtensions
	test al, al
	jz   HaltSystem

	push HDD_ID
	push START_SCRATCH
	push DWORD START_KERNEL
	push DWORD SIZE_KERNEL/SECTOR_SIZE
	push DWORD START_KERNEL_DISK
	call BIOS_ReadFromDiskToHighMemory
	test al, al
	jnz  EnterProtectedMode

	
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
%include "x86/boot/src/biosio.asm"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	
; Enter protected mode 
	
EnterProtectedMode:
	
	mov eax, cr0
	or  eax, 1
	mov cr0, eax

	jmp SEG32_CODE:InProtectedMode                      ; Make a far jump this time to update the code segment to 32-bit


; We are now firmly 32-bit protected mode territory

BITS 32

InProtectedMode:

	mov ax, SEG32_DATA                                  ; Lets set up the segment registers correctly
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	mov eax, MULTIBOOT_MAGIC
	mov ebx, MULTIBOOT_INFO_ADDRESS
	
	jmp START_KERNEL+MULTIBOOT_HEADER_SIZE              ; Launch into the kernel

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


