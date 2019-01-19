BITS 16

; SwitchOnA20 : Enable the A20 line. 
; First the code checks if the line is already enabled
; If the line is enabled it returns
; Otherwise it tries to enable it using BIOS
; It checks again if the line is enabled
; If the line is enabled it returns
; Otherwise it tries to enable it using the PS/2 (8042) controller 
; It checks again if the line is enabled
; If the line is enabled it returns
; Otherwise it tries to enable it using the Fast A20 gate
; It checks again if the line is enabled
; If the line is enabled it returns
; Otherwise, it halts the computer

SwitchOnA20:
	push ax                                           ; Push all general purpose registers to the stack

    Call CheckA20                                     ; Check if A20 line is enabled
    cmp ax, 0                                         ; If AX is 0, then the A20 line is disabled
    jne .a20enabled

    Call EnableA20BIOS                                ; Enable the A20 line using BIOS
    Call CheckA20                                     ; Check again and print the A20 line status
    cmp ax, 0                                         ; Check again
    jne .a20enabled

    Call EnableA20Keyboard                            ; Enable the A20 line using the PS/2 (8042) controller
    Call CheckA20                                     ; Check again and print the A20 line status
    cmp ax, 0                                         ; Check again
    jne .a20enabled

    Call EnableA20FastGate                            ; Enable the A20 line using the Fast A20 Gate -- enable bit 2 on port 0x92
    Call CheckA20                                     ; Check again and print the A20 line status
    cmp ax, 0                                         ; Check again
    jne .a20enabled

	hlt                                               ; If the A20 is still not enabled stop here -- this should be refined

	.a20enabled:
	pop ax                                            ; Reload all the general purpose registers
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
    push ax                                           ; Push all general purpose registers to the stack
    push ds                                           ; Push the data segments as well
    push es
	push di                                           ; Not sure why these 
    push si                                           ; two pushes are needed
 
    xor ax, ax                                        ; ax = 0x0000
    mov es, ax                                        ; es = 0x0000
 
    not ax                                            ; ax = 0xFFFF
    mov ds, ax                                        ; ds = 0xFFFF
 
    mov di, 0x0500                                    ; ES:DI = 0x0000:0x0500
    mov si, 0x0510                                    ; DS:SI = 0xFFFF:0x0510
	                                                  ; If A20 line is disabled both these addresses will point to the same physical location
 
    mov al, byte [es:di]                              ; Push the contents of ES:DI to stack
    push ax
 
    mov al, byte [ds:si]                              ; Push the contents of ES:DI to stack
    push ax
 
    mov byte [es:di], 0x00                            ; Store 0x00 at ES:DI
    mov byte [ds:si], 0xFF                            ; Store 0xFF at DS:SI
	                                                  ; If A20 line is disabled both ES:DI and DS:SI will contain 0xFF
 
    cmp byte [es:di], 0xFF                            ; Check if ES:DI contains 0xFF i.e. if A20 line is disabled
 
    pop ax                                            ; We are done, so restore the contents of ES:DI and DS:SI
    mov byte [ds:si], al
 
    pop ax
    mov byte [es:di], al
 
    mov ax, 0                                         
    je .checka20done                                  ; If A20 line is disabled store 0 in AX, else store 1 in AX
 
    mov ax, 1
 
	.checka20done:
	pop si
	pop di
    pop es                                            ; Reload the data segments
    pop ds
    pop ax                                            ; Reload all general purpose registers
 
    ret





; EnableA20BIOS : Enable the A20 line using INT 15H
; There are several options to try in case this does not work. We will stick with this one for now, and develop further later on. 

EnableA20BIOS:
	push ax

	mov ax, 0x2401                                    ; This is the parameter to INT 15H to enable the A20 line. 
	int 0x15

	pop ax
	ret


; EnableA20Keyboard : Enable the A20 line the 8042 controller for the PS/2 keyboard
; The first byte of the PS/2 controller output controls the A20 gate
; We will need to enable this bit
; First we disable the PS/2 ports (this operation does not have anything to do with the PS/2 ports)
; Then we read out the PS/2 controller output port and store the byte
; We then write back the same byte setting the A20 bit

IOPORT_8042_DATA              equ 0x60                ; PS/2 controller (8042) data port
IOPORT_8042_COMD              equ 0x64                ; PS/2 controller (8042) command/status port
 
COMD_8042_DISABLE_PS2_PORT1   equ 0xAD                ; Command for disabling the first  PS/2 port
COMD_8042_ENABLE_PS2_PORT1    equ 0xAE                ; Command for enabling  the first  PS/2 port
COMD_8042_DISABLE_PS2_PORT2   equ 0xA7                ; Command for disabling the second PS/2 port
COMD_8042_ENABLE_PS2_PORT2    equ 0xA8                ; Command for enabling  the second PS/2 port
COMD_8042_READ_CONT_OUT_PORT  equ 0xD0                ; Command to read out the PS/2 controller output port
COMD_8042_WRITE_CONT_OUT_PORT equ 0xD1                ; Command to write to the PS/2 controller output port

TEST_8042_READ_ACCESS         equ 0x01                ; We always test if the 1st bit of the byte read out from the PS/2 controller command port is 1 -- this tells we have  read access to the PS/2 data port
TEST_8042_WRITE_ACCESS        equ 0x02                ; We always test if the 2nd bit of the byte read out from the PS/2 controller command port is 0 -- this tells we have write access to the PS/2 ports

A20_BIT_ENABLE                equ 0x02                ; The 2nd bit of the data from the PS/2 controller output port drives the A20 gate

EnableA20Keyboard:

		cli

		push ax
	
		; Disbale PS/2 port 1
        call    WriteWaitFor8042                      
        mov     al, COMD_8042_DISABLE_PS2_PORT1      
        out     IOPORT_8042_COMD, al
 
		; Disbale PS/2 port 2
        call    WriteWaitFor8042                      
        mov     al, COMD_8042_DISABLE_PS2_PORT2      
        out     IOPORT_8042_COMD, al
 
		; Command to read from PS/2 controller output port
        call    WriteWaitFor8042
        mov     al, COMD_8042_READ_CONT_OUT_PORT
        out     IOPORT_8042_COMD, al
 
		; Read PS/2 controller output port
        call    ReadWaitFor8042
        in      al, IOPORT_8042_DATA
        push    ax
 
		; Command to write to PS/2 controller output port
        call    WriteWaitFor8042
        mov     al, COMD_8042_WRITE_CONT_OUT_PORT
        out     IOPORT_8042_COMD, al
 
		; Enable A20 bit
        call    WriteWaitFor8042
        pop     ax
        or      al, A20_BIT_ENABLE
        out     IOPORT_8042_DATA, al
 
		; Enable PS/2 port 1
        call    WriteWaitFor8042
        mov     al, COMD_8042_ENABLE_PS2_PORT1
        out     IOPORT_8042_COMD, al
 
		; Enable PS/2 port 2
        call    WriteWaitFor8042                      
        mov     al, COMD_8042_ENABLE_PS2_PORT2      
        out     IOPORT_8042_COMD, al
 
		pop ax
		
		sti

        ret
 

; ReadWaitFor8042 : Function to poll the 8042 status register to check if we have read access to the data port 

ReadWaitFor8042:
        in      al, IOPORT_8042_COMD
        test    al, TEST_8042_READ_ACCESS
        jz      ReadWaitFor8042
        ret

; ReadWaitFor8042 : Function to poll the 8042 status register to check if we have write access to the data and command ports 

WriteWaitFor8042:
        in      al, IOPORT_8042_COMD
        test    al, TEST_8042_WRITE_ACCESS
        jnz     WriteWaitFor8042
        ret



; EnableA20FastGate : Enable the A20 line using the 'Fast A20 Gate'

EnableA20FastGate:

	cli

	in al, 0x92
	or al, 2
	out 0x92, al	

	sti


 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



