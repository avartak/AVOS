; This is the code of the kernel of AVOS
; It displays a welcome message on the screen and then halts the system
; The kernel is to be placed at physical memory location 0x7E00 - so right after the bootloader
; In the floppy image as well the kernel come immediately after the boot sector

ORG 0x7C00

; We are still in real mode
BITS 16

; The first 512 bytes will be replaced in the disk image by the bootloader code
times 512 db 0

; Starting point of the kernel

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Kernel:

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
                xor ax, ax                        ; Clear the AX register
                stosw                             ; Store the contents of the AX register at ES:DI, and increment the value of DI
                loop .clearscreen                 ; Loop as long as CX is non-zero (CX is decremented everytime loop is executed)

        mov di, 0                                 ; Having cleared the screen, lets move to the beginning of the screen and display the welcome message
        mov si, Welcome_Message                   ; Load the string pointer into the SI register
        .printchar:
                lodsb                             ; Load the byte from the address in SI into AL, then increment the address in SI
                cmp al, 0                         ; Have we reached the end of the string ?
                je .done                          ; If so, break out of the loop
                mov ah, 0x0A                      ; We want to print characters in bright green - this is the color code
                stosw                             ; Store the AX register (color code + character) into the video buffer
                jmp .printchar                    ; Continue the printing loop
        .done:                                    ; Printing is done

        pop es                                    ; Restore the ES register back to its original segment address
        popa                                      ; Restore the original state of the general-purpose registers
        ret                                       ; Return control to the kernel


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; The welcome message - It's a C-style string that is terminated by a 0-byte

Welcome_Message db 'Welcome to AVOS!', 0

; Adding a zero padding to the boot sector

times 1024-($-$$) db 0 

