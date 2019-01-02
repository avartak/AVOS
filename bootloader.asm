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

	; The following code can be thought of as a code module that prints a string to screen
	; We will use the BIOS interrupt service routine 0x10 to do the printing
	
	mov si, Welcome_Message	                  ; Load the string pointer into the SI register
	mov ah, 0Eh                               ; We want a teletype output - printing basic characters to the screen 
	.printchar:                               ; We set up a loop here to print every single character
		lodsb                                 ; Load the byte from the address in SI into AL, then increment the address in SI
		cmp al, 0                             ; Have we reached the end of the string ?
		je .done                              ; If so, break out of the loop
		int 10h                               ; Call the BIOS routine 0x10 to print the character stored in AL 
		jmp .printchar                        ; Continue the printing loop
	
	.done:                                    ; Printing is done
	
	; The OS has performed its goal in life
	
	cli                                       ; Clear all interrupts so that we won't be disturbed            
	hlt                                       ; Halt the system	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; The welcome message - It's a C-style string that is terminated by a 0-byte

Welcome_Message db 'Welcome to AVOS!', 0

; Adding a zero padding to the boot sector

times 510-($-$$) db 0 

; The last two bytes of the boot sector need to have the following boot signature 
; Otherwise BIOS will not recognize this as a boot sector

dw 0xAA55

