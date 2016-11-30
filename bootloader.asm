; We are creating a very simple operating system - AVOS v0.2.0 
; It's aim in life is to print a welcome message and then halt the system
; We start with the bootloader - and a very basic one
; Let us recall how we get here :
; - The PC BIOS initializes the machine
; - It loads the boot sector (first 512 bytes) of the boot device into memory
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
	; We will do that later
	
	; Now, we will just initialize the registers DS, ES to 0 

	mov ax, 0
	mov ds, ax
	mov es, ax
	
	; We should also create a stack 
	; Our OS is quite small, so let us set up the stack at 0x8000
	; Let us be generous and allocate 4 KB to the stack
	; This means putting 4096 in the stack pointer SP - remember the stack grows downwards 

	mov ax, 0x8000
	mov ss, ax
	mov sp, 4096

	; We will now load the kernel to memory
	; We adopt an overly simplistic approach
	; We assume that 
	; 1) a floppy disk is being used for booting
	; 2) the kernel image is sitting in the second sector of that disk 
	; We will use the BIOS interrupt routine 0x13 to load this sector at memory location 0x7E00 (just following this bootloader)
	; We will first read the drive parameters using the 'GetDriveInfo' function
	; We will then read the kernel from disk using the 'LoadKernel' function 
	; Once the kernel is loaded to memory we will transfer control to it with a simple jump

	call GetDriveInfo
	mov si, Kernel_Start                      ; LoadKernel function will copy the kernel image from disk to ES:SI
	call LoadKernel
	jmp Kernel_Start                          ; Jump to the location of the kernel in memory 

; The GetDriveInfo function - obtain the drive parameters

GetDriveInfo:
        pusha                                     ; We start by pushing all the general-purpose registers onto the stack
	                                          ; This way we can restore their values after the function returns 

	; Get drive parameters so that we can read the kernel image
	push es
        mov ax, 0
	mov es, ax
        mov di, ax                                ; It is recommended we set ES:DI to 0:0 to work around some buggy BIOS
        mov ah, 8                                 ; AH=8 tells the BIOS to read the drive parameters
        int 0x13                                  ; INT 0x13 is all about I/O from disks
        and cx, 0x3F                              ; Bits 0-5 of CX store the number of sectors per track
        mov [Sectors_Per_Track], cx               ; Store this information in a variable
        add dh, 1                                 ; Number of heads is stored in DH (numbering starts at 0, hence the increment)
        mov [Sides], dh                           ; Store this information in a variable

	pop es                                    ; Restore the ES register to its original state
        popa                                      ; Restore the original state of the general-purpose registers
        ret                                       ; Return control to the bootloader

; The LoadKernel function - load kernel image from disk to memory (ES:SI to be specific)
; We are missing some functionality here
; What if we encounter problems reading from the disk ? There should be some protection for that
; Also, who uses floppy disks anymore ? :) But ok, baby steps ...

LoadKernel:
        pusha                                     ; Push all the general-purpose registers onto the stack

        ; Reading the kernel image from disk (The kernel is just one sector long for now)
	; When need to use the CHS scheme when using INT 0x13 to read from disk
	; The following relations give the conversion from LBA to CHS
	; Temp     = LBA / (Sectors per Track) 
	; Sector   = (LBA % (Sectors per Track)) + 1
	; Head     = Temp % (Number of Heads)
	; Cylinder = Temp / (Number of Heads) 

        mov ax, 1                                 ; Logical block address (LBA) of the sector to be read - starts from 0
        mov dx, 0                                 ; 
        div word [Sectors_Per_Track]              ; Division by a word --> dividend is DX (MSB) : AX (LSB)
	                                          ; Result : Quotient --> AX ; Remainder --> DX						
	add dx, 1                                 ; Sectors start at 1, hence we need to increment the remainder 
        mov cl, dl                                ; Sector number on the track should be stored in CL
        mov dx, 0                                 
        div word [Sides]                          ; Divide the quotient (number of tracks) by the number of heads
        mov ch, al                                ; The quotient is the cylinder which goes in CH
        mov dh, dl                                ; The remainder is the head which should go in DH

        mov dl, byte [bootdev]                    ; Drive ID in dl

        mov bx, si                                ; INT 0x13 will load the data from the disk at ES:BX
        mov ah, 2                                 ; AH=2 tells the BIOS to read from the disk
        mov al, 1                                 ; AL contains the numbers of sectors to be read out
        int 0x13                                   

        popa                                      ; Restore the original state of the general-purpose registers
        ret                                       ; Return control to the bootloader


Sectors_Per_Track dw 18
Sides             db 2
bootdev           db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Adding a zero padding to the boot sector

times 510-($-$$) db 0 

; The last two bytes of the boot sector need to have the following boot signature 
; Otherwise BIOS will not recognize this as a boot sector

dw 0xAA55

; Start of the kernel code 
Kernel_Start:
