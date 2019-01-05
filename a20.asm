[BITS 16]

Switch_On_A20:
	pusha

    Call Check_A20                            ; Check if A20 line is enabled
    cmp ax, 0                                 ; If AX is 0, then the A20 line is disabled
    jne .a20enabled

    Call Enable_A20_BIOS                      ; Enable the A20 line

    Call Check_A20                            ; Check again and print the A20 line status
    cmp ax, 0                                 ; Check again
    jne .a20enabled

	hlt                                       ; If the A20 is still not enabled stop here -- this should be refined

	.a20enabled:
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



