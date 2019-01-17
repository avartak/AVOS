BITS 16

; 16-bit code for some very functions to display text on screen
; First the clear screen function

ClearScreen:
	push bp
	mov bp, sp

	pusha
	push es

	mov ax, 0xB800
	mov es, ax                                ; We set the ES stack to the start of the video memory
	mov di, 0                                 ; We start writing from the beginning of the video buffer
	mov ax, 80                                ; There are 80 columns
	mov bx, 25                                ; And 25 rows
	mul bx                                    ; Total number of characters we can print: 80x25
	mov cx, ax  
	
	.cls:
		xor ax, ax                            ; Clear the AX register
		stosw                                 ; Store the contents of the AX register at ES:DI, and increment the value of DI
		loop .cls                             ; Loop as long as CX is non-zero (CX is decremented everytime loop is executed)
	

	pop es
	popa

	mov sp, bp
	pop bp
	ret 

PrintLine:
	push bp
	mov bp, sp

	pusha
	push es

	mov ax, 0xB800
	mov es, ax                                ; We set the ES stack to the start of the video memory
	mov di, [bp+6]                            ; Lets load the line number in di
	mov ax, 160                               ; Each line is 80 characters long and each character is represented by two bytes
	mul di
	mov di, ax
	mov si, [bp+4]                            ; Load the string pointer into the SI register

	.printchar:
		lodsb                                 ; Load the byte from the address in DS:SI into AL, then increment the address in SI
		cmp al, 0                             ; Have we reached the end of the string ?
		je .done                              ; If so, break out of the loop
		mov ah, 0x0F                          ; We want to print characters in bright green - this is the color code
		stosw                                 ; Store the AX register (color code + character) into the video buffer
		jmp .printchar                        ; Continue the printing loop
	.done:	

	mov al, [Current_Screen_Line]
	inc al
	mov  [Current_Screen_Line], al

	pop es
	popa

	mov sp, bp
	pop bp
	ret 


Current_Screen_Line   db 0           
