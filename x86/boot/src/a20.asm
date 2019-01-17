BITS 16

; SwitchOnA20 : Enable the A20 line. 
; First the code checks if the line is already enabled
; If not, it tries to enable it
; It checks again if the line is enabled
; If the line is enabled it returns
; Otherwise, it halts the computer
; There needs to be better exception handling introduced here, something to be improved later

SwitchOnA20:
	pusha                                     ; Push all general purpose registers to the stack

    Call CheckA20                             ; Check if A20 line is enabled
    cmp ax, 0                                 ; If AX is 0, then the A20 line is disabled
    jne .a20enabled

    Call EnableA20BIOS                        ; Enable the A20 line

    Call CheckA20                             ; Check again and print the A20 line status
    cmp ax, 0                                 ; Check again
    jne .a20enabled

	hlt                                       ; If the A20 is still not enabled stop here -- this should be refined

	.a20enabled:
	popa                                      ; Reload all the general purpose registers
	ret


; The following code is public domain licensed
 
; CheckA20 : Check the status of the A20 line in a completely self-contained state-preserving way.
; The function can be modified as necessary by removing push's at the beginning and their
; respective pop's at the end if complete self-containment is not required.
;
; Returns: 
; 0 in ax if the A20 line is disabled (memory wraps around)
; 1 in ax if the A20 line is enabled (memory does not wrap around)



CheckA20:
    pusha                    ; Push all general purpose registers to the stack
    push ds                  ; Push the data segments as well
    push es
	push di                  ; Not sure why these 
    push si                  ; two pushes are needed
 
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
    je .checka20done         ; If A20 line is disabled store 0 in AX, else store 1 in AX
 
    mov ax, 1
 
	.checka20done:
	pop si
	pop di
    pop es                   ; Reload the data segments
    pop ds
    popa                     ; Reload all general purpose registers
 
    ret





; EnableA20BIOS : Enable the A20 line using INT 15H
; There are several options to try in case this does not work. We will stick with this one for now, and develop further later on. 

EnableA20BIOS:
	push ax
	mov ax, 0x2401            ; This is the parameter to INT 15H to enable the A20 line. 
	int 0x15
	pop ax
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



