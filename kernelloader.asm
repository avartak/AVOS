; This is the code of the kernel of AVOS
; There is no code in this kernel. It only contains a C-style string of the message to the displayed on screen
; The kernel is to be placed at physical memory location 0x7E00 - so right after the bootloader
; In the floppy image as well the kernel come immediately after the boot sector

; We are still in real mode
[BITS 16]

; Starting point of the kernel loader

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Kernel_Loader:

	; The kernel loader loads the GDT
	; It then switches to the unreal mode (protected mode -> segment register manipulation -> real mode)
	; The kernel is loader to 0x100000 and launched
	; The kernel prints a welcome message

	cli                                       ; Clear all interrupts so that we won't be disturbed            

	call Switch_On_A20                        ; Check and enable A20

	lgdt [GDT_Desc]                           ; Load the GDT

	mov eax, cr0                              ; Enter protected mode
	or eax, 1
	mov cr0, eax

	jmp EnterUnreal                           ; Make a near jump -- the CS does not switch to protected mode (Big Unreal)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;






; Code to enable the A20 line
%include "a20.asm"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;







; Global Descriptor Table
; 8-byte entries for each segemnt type
; First entry is default, contains all zeroes
; For details on segment descriptors see : https://wiki.osdev.org/Global_Descriptor_Table

GDT_Start:
GDT_Null:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 0x00
    db 0x00
    db 0x00
GDT_Code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00
GDT_Data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00
GDT_Stack:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x96
    db 0xCF
    db 0x00
GDT_End:

GDT_Desc:
   dw GDT_End - GDT_Start - 1
   dd GDT_Start

CODE_SEG   equ GDT_Code   - GDT_Start
DATA_SEG   equ GDT_Data   - GDT_Start
STACK_SEG  equ GDT_Stack  - GDT_Start

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



; Entering unreal mode

EnterUnreal:

	mov bx, DATA_SEG                          ; We will update the DS and ES segment register to 32-bit mode
	mov ds, bx                                ; The MOV will also load the protected mode segment descriptor
	mov es, bx                                ; On switching back to real mode the descriptor (and hence the register limit, size, etc.) will stay as is

	and al,0xFE                               ; Switch back to real mode
	mov cr0, eax

	mov ax, 0                                 ; Set the DS, ES segment address to 0x0
	mov ds, ax
	mov es, ax

	sti                                       ; Enable interrupts so that we can use the BIOS routines

    call GetFloppyInfo                        ; Load the kernel at 0x100000
	mov ax, 3
	mov dl, 1
	mov esi, 0x8000
	mov edi, 0x100000
    call ReadAndMove	

    cli                                       ; Clear all interrupts as we now enter the protected mode for good           

    mov eax, cr0                              ; Enter protected mode
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:Enter32                      ; Make a far jump this time to update the code segment to 32-bit 

[BITS 32]

Enter32:

    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ax, STACK_SEG
    mov ss, ax

	jmp CODE_SEG:0x100000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



