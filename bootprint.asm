[BITS 16]

; Function: Clear_Screen_Boot
; Clears the video buffer
; 16-bit code used for the boot loader

Clear_Screen_Boot:
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

    pop es                                    ; Restore the ES register back to its original segment address
    popa                                      ; Restore the original state of the general-purpose registers
    ret                                       ; Return control to the kernel




; Function: Print_Boot
; Displays a string whose addess is in SI on screen
; We will write directly to the video buffer
; We will write to the line number stored in dl
; 16-bit code used for the boot loader

Print_Boot:
	pusha                                     ; We start by pushing all the general-purpose registers onto the stack
	                                          ; This way we can restore their values after the function returns 
    push es                                   ; Push the ES register onto the stack - we will use it to access the video memory
    mov ax, 0xB800                            ; In the PC architecture the video buffer sits at 0xB8000
    mov es, ax                                ; We put the ES segment at that location

	mov ax, 0
	mov al, dl                                ; The line number is stored in dl
	mov bx, 160                               ; Number of columns per line
	mul bx                                    ; Get the character position to print the line
	mov di, ax                                ; Store the character position in DI
	
	.printchar:
	    lodsb                                 ; Load the byte from the address in SI into AL, then increment the address in SI
	    cmp al, 0                             ; Have we reached the end of the string ?
	    je .done                              ; If so, break out of the loop
	    mov ah, cl                            ; Font attribute is stored in cl
	    stosw                                 ; Store the AX register (color code + character) into the video buffer
	    jmp .printchar                        ; Continue the printing loop
	.done:                                    ; Printing is done
	
	pop es                                    ; Restore the ES register back to its original segment address
	popa                                      ; Restore the original state of the general-purpose registers
	ret                                       ; Return control to the kernel


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;










