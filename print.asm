
[BITS 32]

; Function: Clear_Screen
; Clears the video buffer

Clear_Screen:
    pusha                                     ; We start by pushing all the general-purpose registers onto the stack
                                              ; This way we can restore their values after the function returns 

	mov eax, 0xB8000
    mov edi, eax                              ; We start writing from the beginning of the video buffer
    mov ecx, 0x7D0                            ; 80x25 --> Number of characters displayed on the teletype

    .clearscreen:
        xor ax, ax                            ; Clear the AX register
        stosw                                 ; Store the contents of the AX register at ES:DI, and increment the value of DI
		loop .clearscreen

    popa                                      ; Restore the original state of the general-purpose registers
    ret                                       ; Return control to the kernel




; Function: Print
; Displays a string whose addess is in SI on screen
; We will write directly to the video buffer
; We will write to the line number stored in dl

Print:
    pusha                                     ; We start by pushing all the general-purpose registers onto the stack
                                              ; This way we can restore their values after the function returns 
	mov eax, 0
    mov al, dl                                ; The line number is stored in dl
    mov bx, 160                               ; Number of columns per line
    mul bx                                    ; Get the character position to print the line
	mov ebx, 0xB8000                          ; Address of the video buffer
	add eax, ebx
    mov edi, eax                              ; Store the character position in DI
	mov eax, 0

    .printchar:
        lodsb                                 ; Load the byte from the address in SI into AL, then increment the address in SI
        cmp al, 0                             ; Have we reached the end of the string ?
        je .done                              ; If so, break out of the loop
	    mov ah, cl                            ; Font attribute is stored in cl
        stosw                                 ; Store the AX register (color code + character) into the video buffer
        jmp .printchar                        ; Continue the printing loop
    .done:                                    ; Printing is done

    popa                                      ; Restore the original state of the general-purpose registers
    ret                                       ; Return control to the kernel




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;








