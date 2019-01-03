; This is the code of the kernel of AVOS
; There is no code in this kernel. It only contains a C-style string of the message to the displayed on screen
; The kernel is to be placed at physical memory location 0x7E00 - so right after the bootloader
; In the floppy image as well the kernel come immediately after the boot sector

ORG 0x7C00

; We are still in real mode
[BITS 16]

; The first 512 bytes will be replaced in the disk image by the bootloader code
times 512 db 0

; Starting point of the kernel

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Kernel:

	; This kernel prints a welcome message and then tests the status of the A20 line
	; If the A20 line is disabled it makes one attempt to enable it and the tests again
	; We first clear the screen by calling the 'Clear_Screen' function
	; We then call the 'Print' function to display our welcome message
	; We then test the status of the A20 line and print it on the screen
	; After this the OS has performed its goal in life

	cli                                       ; Clear all interrupts so that we won't be disturbed            

	lgdt [GDT_Desc]                           ; Load the GDT

	mov eax, cr0                              ; Enter protected mode
	or eax, 1
	mov cr0, eax

	jmp CODE_SEG:Enter32                      ; Make a far jump

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

CODE_SEG  equ GDT_Code  - GDT_Start
DATA_SEG  equ GDT_Data  - GDT_Start
STACK_SEG equ GDT_Stack - GDT_Start

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;




%include "print.asm"




; Entering protected mode from a far jump

[BITS 32]

Enter32:
	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ax, STACK_SEG
	mov ss, ax

	mov esp, 0x90000

	mov cl, 0x0A
	inc dl
	mov esi, PM_Switch_Message
	call Print
	
	mov cl, 0x04
	inc dl
	inc dl
	mov esi, Welcome_Message_Ln1
	call Print
	inc dl
	mov esi, Welcome_Message_Ln2
	call Print
	inc dl
	mov esi, Welcome_Message_Ln3
	call Print
	inc dl
	mov esi, Welcome_Message_Ln4
	call Print
	inc dl
	mov esi, Welcome_Message_Ln5
	call Print
	inc dl
	mov esi, Welcome_Message_Ln6
	call Print

	hlt                                       ; Halt the system	




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; Message --> Protected mode switch
PM_Switch_Message db '[OK]      Switched to protected mode', 0

; Welcome message for the OS
Welcome_Message_Ln1 db '                           ___  _   _ _____ _____   _ ', 0
Welcome_Message_Ln2 db '                          / _ \| | | |  _  /  ___| | |', 0
Welcome_Message_Ln3 db '                         / /_\ \ | | | | | \ `--.  | |', 0
Welcome_Message_Ln4 db '                         |  _  | | | | | | |`--. \ | |', 0
Welcome_Message_Ln5 db '                         | | | \ \_/ | \_/ /\__/ / |_|', 0
Welcome_Message_Ln6 db '                         \_| |_/\___/ \___/\____/  (_)', 0
                                                                                       


; Adding a zero padding to the boot sector
times 1536-($-$$) db 0 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

