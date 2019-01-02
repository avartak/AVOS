; This is the code of the kernel of AVOS
; There is no code in this kernel. It only contains a C-style string of the message to the displayed on screen
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

	; This kernel prints a welcome message and then tests the status of the A20 line
	; If the A20 line is disabled it makes one attempt to enable it and the tests again
	; We first clear the screen by calling the 'Clear_Screen' function
	; We then call the 'Print' function to display our welcome message
	; We then test the status of the A20 line and print it on the screen
	; After this the OS has performed its goal in life

	call Clear_Screen                         ; Clears screen
	mov dl, 0                                 ; On which line to print a message
	mov si, Welcome_Message                   ; Load the pointer to the welcome message into SI
	call Print

	cli                                       ; Clear all interrupts so that we won't be disturbed            

	call Switch_On_A20                        ; Check and enable A20

	hlt                                       ; Halt the system	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;










; Function: Clear_Screen
; Clears the video buffer

Clear_Screen:
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




; Function: Print
; Displays a string whose addess is in SI on screen
; We will write directly to the video buffer
; We will write to the line number stored in dl

Print:
	pusha                                     ; We start by pushing all the general-purpose registers onto the stack
	                                          ; This way we can restore their values after the function returns 
    push es                                   ; Push the ES register onto the stack - we will use it to access the video memory
    mov ax, 0xB800                            ; In the PC architecture the video buffer sits at 0xB8000
    mov es, ax                                ; We put the ES segment at that location

	mov ax, 0
	mov al, dl                                ; The line number is stored in dl
	mov bx, 80                                ; Number of columns per line
	mul bx                                    ; Get the character position to print the line
	mov di, ax                                ; Store the character position in DI
	
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






; Function: Switch_On_A20
; Checks if A20 line is enabled, if not, tries to enable it once, and then checks again

Switch_On_A20:
	pusha

    Call Check_A20                            ; Check if A20 line is enabled
    cmp ax, 0                                 ; If AX is 0, then the A20 line is disabled
    jne .printA20enabled
    mov si, A20_Disabled_Message
    inc dl
    inc dl
    call Print

    mov si, A20_Enabling_Message              ; Print a message saying that we are trying to enable the A20 line
    inc dl
    inc dl
    call Print
    call Enable_A20_BIOS

    Call Check_A20                            ; Check again and print the A20 line status
    cmp ax, 0
    jne .printA20enabled
    mov si, A20_Enabled_Message
    inc dl
    inc dl
    call Print

    .printA20enabled:                         ; Print a message that the A20 line is enabled
        mov si, A20_Enabled_Message
        inc dl
        inc dl
        call Print

	popa
	ret

; The following code is public domain licensed
 
; Function: Check_A20
;
; Purpose: to check the status of the A20 line in a completely self-contained state-preserving way.
;          The function can be modified as necessary by removing push's at the beginning and their
;          respective pop's at the end if complete self-containment is not required.
;
; Returns: 0 in ax if the A20 line is disabled (memory wraps around)
;          1 in ax if the A20 line is enabled (memory does not wrap around)



Check_A20:
    pushf
    push ds
    push es
    push di
    push si
 
    xor ax, ax               ; ax = 0x0000
    mov es, ax               ; es = 0x0000
 
    not ax                   ; ax = 0xFFFF
    mov ds, ax               ; ds = 0xFFFF
 
    mov di, 0x0500           ; ES:DI = 0x0000:0x0500
    mov si, 0x0510           ; DS:SI = 0xFFFF:0x0510
	                         ; If A20 line is disabled both these addresses will point to the same physical location
 
    mov al, byte [es:di]     ; Push the contents of ES:DI to stack
    push ax
 
    mov al, byte [ds:si]     ; Push the contents of ES:DI to stack
    push ax
 
    mov byte [es:di], 0x00   ; Store 0x00 at ES:DI
    mov byte [ds:si], 0xFF   ; Store 0xFF at DS:SI
	                         ; If A20 line is disabled both ES:DI and DS:SI will contain 0xFF
 
    cmp byte [es:di], 0xFF   ; Check if ES:DI contains 0xFF i.e. if A20 line is disabled
 
    pop ax                   ; We are done, so restore the contents of ES:DI and DS:SI
    mov byte [ds:si], al
 
    pop ax
    mov byte [es:di], al
 
    mov ax, 0                
    je .check_a20__exit      ; If A20 line is disabled store 0 in AX, else store 1 in AX
 
    mov ax, 1
 
	.check_a20__exit:
    pop si
    pop di
    pop es
    pop ds
    popf
 
    ret





; Function to enable the A20 line

Enable_A20_BIOS:
	push ax
	mov ax, 0x2401            ; This is the interrupt to enable the A20 line. 
	int 0x15
	pop ax
	ret



; Welcome message of the kernel
Welcome_Message      db 'Welcome to AVOS!', 0

; Message saying A20 line is disabled
A20_Enabled_Message  db 'A20 line is enabled', 0

; Message saying A20 line is enabled
A20_Disabled_Message db 'A20 line is disabled', 0

; Message saying A20 line enabling in progess
A20_Enabling_Message db 'Enabling the A20 line', 0

; Adding a zero padding to the boot sector
times 1024-($-$$) db 0 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

