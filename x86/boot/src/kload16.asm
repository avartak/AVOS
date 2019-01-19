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
Kload16:

	; To enable the protected mode we will :
	; 1) Clear all interrupts
	; 2) Switch on the A20 line 
	; 3) Create a GDT and load it
	; 4) Enter protected mode
	; 5) Enter unreal mode
	; 6) Copy the stage-3 boot loader and then the kernel to memory
	; 7) Enter back into 32-bit protected mode
	; Launch the 3rd stage of the boot loader

	call SwitchOnA20                          ; Check and enable A20 ; Code in a20.asm

	call CreateTables                         ; create a blank IDT and a GDT with both kernel and user segments for code, data and stack
 
	cli                                       ; Clear all interrupts so that we won't be disturbed            

	lgdt [GDT_Desc]                           ; Load the GDT -- Note that GDT_Desc is defined in tables.asm

	mov eax, cr0                              ; Enter protected mode
	or  eax, 1
	mov cr0, eax


	; Entering the Unreal mode

	mov bx, SEG_DS32                          ; We will update the DS and ES segment register to 32-bit mode
	mov ds, bx                                ; The MOV will also load the protected mode segment descriptor
	mov bx, SEG_ES32                          ; On switching back to real mode the descriptor (and hence the register limit, size, etc.) will stay as is
	mov es, bx      

	mov eax, cr0	
	and al,0xFE                               ; Switch back to real mode
	mov cr0, eax
	
	mov ax, 0                                 ; Set the DS, ES segment address to 0x0
	mov ds, ax
	mov es, ax
	
	sti                                       ; Enable interrupts so that we can use the BIOS routines
	
	mov  dl, FLOPPY_ID                        ; We are reading from a floppy disk 
	call ReadDriveParameters                  ; Load the 3rd stage of the boot loader and the kernel

	mov  dl, FLOPPY_ID
	mov  ax, START_BOOT3_DISK                 ; Copy data starting from 12 KB logical address of the disk
	mov  bl, SIZE_BOOT3_DISK                  ; Copy 8 KB of data (16 sectors)
	mov  si, START_BOOT3
	call ReadSectorsFromDrive                 ; Copy the 3rd stage of the bootloader

	mov  dl, FLOPPY_ID
	mov  cx, KERNL_COPY_ITER                  ; Number of iterations of read-and-move (each iteration as can be seen below moves 64 KB of data)
	mov  ax, START_KERNL_DISK                 ; Copy kernel starting from 20 KB logical address of the disk
	mov  bl, SECTRS_PER_ITER                  ; Copy 64 KB of data (128 sectors) from the disk
	mov  si, START_SCRCH                      ; Temporary pool to hold each sector before copying it to the high memory
	mov edi, START_KERNL                      ; Starting point in memory of the kernel
	                                          ; We are moving 1 MB of data from the disk as kernel
	.iterateReadAndMove:
		call ReadAndMove	                  ; Copy kernel from disk and move to high memory (1 MB)
		loop .iterateReadAndMove

	; Entering back into the protected mode 
	
	cli                                       ; Clear all interrupts as we now enter the protected mode for good           
	
	mov eax, cr0                              ; Enter protected mode
	or eax, 1
	mov cr0, eax

	jmp SEG_CS32:START_BOOT3                  ; Make a far jump this time to update the code segment to 32-bit, and launch into the 3rd stage of the boot loader 


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; Code to enable the A20 line
%include "x86/boot/src/a20.asm"

; Load the tables at designated locations in memory
%include "x86/boot/src/tables.asm"

; ReadDriveParameters and ReadSectorsFromDrive are define in this file
%include "x86/boot/src/biosio.asm"



