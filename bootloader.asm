; We are creating a very simple operating system - AVOS
; It's aim in life is to print a welcome message and then halt the system
; We start with the bootloader - and a very basic one
; Let us recall how we get here :
; - The PC BIOS initializes the machine
; - It loads the boot sector (first 512 bytes) of the boot device (in our case a floppy disk) into memory
; - The boot sector is loaded at memory location 0x7C00
; The bootloader is not equipped to perform complicated tasks due to its small size
; It uses the BIOS routines to load the OS kernel in memory and hands over the control of the machine to the kernel
; Our bootloader will load a very rudimentary 16-bit kernel that will print a welcome message on the screen

; We need to tell the assembler the memory location at which the bootloader code will be placed
; This will help the assembler to translate the labels in our code to the correct memory locations

ORG 0x7C00

; We start in the REAL mode - effectively our system resembles the 16-bit 8086 ancestors
; We need to tell the assembler to produce 16-bit instructions

BITS 16

; This is where the bootloader starts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Boot:

	; We should initialize the segment registers to the base-address values that we want to use
	; We should not assume the segment registers to be initialized in a certain way when we are handed control
	; The segment registers known in 16-bit environment are : CS, DS, ES, SS
	; Physical address given by reg:add combination is : [reg] x 0x100 + add
	; This allows us to address a memory range starting from 0 to 0xFFFF00+0xFFFF
	; If you do the math you will find that this range exceeds the 1 MB address space that a 20-bit address bus can physically access
	; By default the addresses beyond the 1 MB mark get looped back to 0
	; However, in reality we now have processors capable of physically accessing addresses beyond 1 MB
	; To enable access to this >1MB address space in real mode, we would need to enable the A20 line
	; Our kernel will, check the status of the A20 line and will try once to enable it in case it is disabled
	
	; Now, we will just initialize the registers DS, ES to 0 

	mov ax, 0
	mov ds, ax
	mov es, ax
	
	; We should also create a stack 
	; Our OS is quite small, so let us set up the stack at 0x8000
	; Let us be generous and allocate 4 KB to the stack
	; This means putting 4096 in the stack pointer SP - remember the stack grows downwards 

	mov ax, 0x9000
	mov ss, ax
	mov sp, 4096

	; Welcome from the boot loader
	call Clear_Screen_Boot                    ; Clears screen
	mov cl, 0x0F                              ; Font attribute -- white color 
	mov dl, 0                                 ; On which line to print a message
	mov si, Boot_Message                      ; Load the pointer to the welcome message into SI
	call Print_Boot
	inc dl

	; We will now load the kernel to memory
	; We adopt an overly simplistic approach
	; We assume that 
	; 1) a floppy disk is being used for booting
	; 2) the kernel image is sitting in the second sector of that disk 
	; We will use the BIOS interrupt routine 0x13 to load this sector at memory location 0x7E00 (just following this bootloader)
	; We will first read the drive parameters using the 'GetDriveInfo' function
	; We will then read the kernel from disk using the 'LoadKernel' function 
	; We will lastly enable the A20 line and jump to the kernel
	; This kernel is our first executable kernel. It prints a welcome message

	call GetDriveInfo
	mov si, Kernel_Start                      ; LoadKernel function will copy the kernel image from disk to ES:SI
	call LoadKernel

	cli                                       ; Clear all interrupts so that we won't be disturbed            
	call Switch_On_A20                        ; Check and enable A20
	sti                                       ; Re-enable interrupts

	jmp Kernel_Start

%include "bootprint.asm"
%include "bootio.asm"
%include "a20.asm"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Welcome message for the OS in real mode
Boot_Message         db '                         Preparting to load the AVOS kernel', 0

; Adding a zero padding to the boot sector

times 510-($-$$) db 0 

; The last two bytes of the boot sector need to have the following boot signature 
; Otherwise BIOS will not recognize this as a boot sector

dw 0xAA55

; Start of the kernel code 
Kernel_Start:
