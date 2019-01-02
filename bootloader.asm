; We are creating a very simple operating system - AVOS v0.0.1 
; This size of this OS is just a few bytes
; It's aim in life is to print a welcome message and then halt the system
; In reality this is a bootloader - and a very basic one at that
; Let us recall how we get here :
; - The PC BIOS initializes the machine
; - It loads the boot sector (first 512 bytes) of the boot device into memory
; - The boot sector is loaded at memory location 0x7C00

; We need to tell the assembler the memory location at which this code will be loaded
; This will help the assembler to translate the labels in our code to the correct memory locations

ORG 0x7C00

; We start in the REAL mode - effectively our system resembles the 16-bit 8086 ancestors
; We need to tell the assembler to produce 16-bit instructions

BITS 16

; This is where our OS starts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Kernel:

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

	; We now call the 'Print' function to display our welcome message (function defined later)
	; After printing the welcome message the OS has performed its goal in life

	call Print
	cli                                       ; Clear all interrupts so that we won't be disturbed            
	hlt                                       ; Halt the system	

; The Print function - displays a string on screen
; We will write directly to the video buffer
; The function first clears the scree, then displays the welcome message

Print:
	pusha                                     ; We start by pushing all the general-purpose registers onto the stack
	                                          ; This way we can restore their values after the function returns 
	push es                                   ; Push the ES register onto the stack - we will use it to access the video memory
	mov ax, 0xB800                            ; In the PC architecture the video buffer sits at 0xB8000
	mov es, ax                                ; We put the ES segment at that location
	mov di, 0                                 ; We start writing from the beginning of the video buffer
	mov ax, 80                                ; There are 80 columns
	mov bx, 25                                ; And 25 rows
	mul bx                                    ; Total number of characters we can print: 80x25
	mov cx, ax                                ; Set this number in the counter register - to be used for clearing the creen

	.clearscreen:
		xor ax, ax                            ; Clear the AX register
		stosw                                 ; Store the contents of the AX register at ES:DI, and increment the value of DI
		loop .clearscreen                     ; Loop as long as CX is non-zero (CX is decremented everytime loop is executed)

	mov di, 0                                 ; Having cleared the screen, lets move to the beginning of the screen and display the welcome message
	mov si, Welcome_Message                   ; Load the string pointer into the SI register
	.printchar:
		lodsb                                 ; Load the byte from the address in SI into AL, then increment the address in SI
		cmp al, 0                             ; Have we reached the end of the string ?
		je .done                              ; If so, break out of the loop
		mov ah, 0x0A                          ; We want to print characters in bright green - this is the color code
		stosw                                 ; Store the AX register (color code + character) into the video buffer
		jmp .printchar                        ; Continue the printing loop
	.done:                                    ; Printing is done

	pop es                                    ; Restore the ES register back to its original segment address
	popa                                      ; Restore the original state of the general-purpose registers
	ret                                       ; Return control to the kernel


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; The welcome message - It's a C-style string that is terminated by a 0-byte

Welcome_Message db 'Welcome to AVOS!', 0

; Adding a zero padding to the boot sector

times 510-($-$$) db 0 

; The last two bytes of the boot sector need to have the following boot signature 
; Otherwise BIOS will not recognize this as a boot sector

dw 0xAA55

