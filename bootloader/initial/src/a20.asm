section .text

BITS 16

; Physical address given by reg:add combination is : [reg] x 0x10 + add
; This allows us to address a memory range starting from 0 to 0xFFFF0+0xFFFF
; If you do the math you will find that this range exceeds the 1 MB address space that a 20-bit address bus (on the original 8086/8088) can physically access
; By default the addresses beyond the 1 MB mark get looped back to 0
; However, in reality we now have processors capable of physically accessing addresses beyond 1 MB
; To enable access to this >1MB address space, we need to enable the A20 line

; A20_Enable : Enable the A20 line. 
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

global A20_Enable
A20_Enable:

	call A20_IsEnabled                                ; Check if A20 line is enabled
	test al, al                                       ; If AX is 0, then the A20 line is disabled
	jnz  .end
	
	call A20_EnableWithBIOS                           ; Enable the A20 line using BIOS
	call A20_IsEnabled                                ; Check again if the A20 line is enabled
	test al, al                                       
	jnz  .end
	
	call A20_EnableWithPS2Keyboard                    ; Enable the A20 line using the PS/2 (8042) controller
	call A20_IsEnabled                                ; Check again if the A20 line is enabled
	test al, al                                        
	jnz  .end
	
	call A20_EnableWithFastGate                       ; Enable the A20 line using the Fast A20 Gate -- enable bit 2 on port 0x92
	call A20_IsEnabled                                ; Check again if the A20 line is enabled
	
	.end:	
	ret


; The following code is public domain licensed
 
; A20_IsEnabled : Check the status of the A20 line in a completely self-contained state-preserving way.
; The function can be modified as necessary by removing push's at the beginning and their
; respective pop's at the end if complete self-containment is not required.
; Returns: 
; 0 in ax if the A20 line is disabled (memory wraps around)
; 1 in ax if the A20 line is enabled (memory does not wrap around)



A20_IsEnabled:
	push di                                           ; Save SI and DI registers
	push si
	push ds                                           ; Save DS and ES registers
	push es
	
	xor  ax, ax                                       ; ax = 0x0000
	mov  es, ax                                       ; es = 0x0000
	not  ax                                           ; ax = 0xFFFF
	mov  ds, ax                                       ; ds = 0xFFFF
	
	mov  di, 0x0500                                   ; ES:DI = 0x0000:0x0500
	mov  si, 0x0510                                   ; DS:SI = 0xFFFF:0x0510
	                                                  ; If A20 line is disabled both these addresses will point to the same physical location
	
	mov  al, [es:di]                                  ; Push the contents of ES:DI to stack
	push ax
	mov  al, [ds:si]                                  ; Push the contents of ES:SI to stack
	push ax
	
	mov  BYTE [es:di], 0x00                           ; Store 0x00 at ES:DI
	mov  BYTE [ds:si], 0xFF                           ; Store 0xFF at DS:SI
	                                                  ; If A20 line is disabled both ES:DI and DS:SI will contain 0xFF
	cmp  BYTE [es:di], 0xFF                           ; Check if ES:DI contains 0xFF i.e. if A20 line is disabled
	
	pop  ax                                           ; We are done, so restore the contents of ES:DI and DS:SI
	mov  [ds:si], al
	pop  ax
	mov  [es:di], al
	
	mov  al, 0                                         
	je   .end                                         ; If A20 line is disabled store 0 in AX, else store 1 in AX
	mov  al, 1
	
	.end:
	pop  es                                           ; Restore the DS and ES registers
	pop  ds
	pop  si                                           ; Restore the SI and DI registers
	pop  di
	ret





; A20_EnableWithBIOS : Enable the A20 line using INT 0x15
; There are several options to try in case this does not work. We will stick with this one for now, and develop further later on. 

A20_EnableWithBIOS:
	pusha

	mov  ax, 0x2401                                    ; This is the parameter to INT 0x15 to enable the A20 line. 
	int  0x15

	popa
	ret


; A20_EnableWithPS2Keyboard : Enable the A20 line the 8042 controller for the PS/2 keyboard
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

TEST_8042_READ_ACCESS         equ 0x01                ; Test if the 1st bit of the byte read out from the PS/2 controller command port is 1 -- we have  read access to the PS/2 data port
TEST_8042_WRITE_ACCESS        equ 0x02                ; Test if the 2nd bit of the byte read out from the PS/2 controller command port is 0 -- we have write access to the PS/2 ports

A20_BIT_ENABLE                equ 0x02                ; The 2nd bit of the data from the PS/2 controller output port drives the A20 gate

A20_EnableWithPS2Keyboard:
	
	; Save the contents of AX
	push ax

	; Disbale PS/2 port 1
	call A20_WriteWaitFor8042                      
	mov  al, COMD_8042_DISABLE_PS2_PORT1      
	out  IOPORT_8042_COMD, al
	
	; Disbale PS/2 port 2
	call A20_WriteWaitFor8042                      
	mov  al, COMD_8042_DISABLE_PS2_PORT2      
	out  IOPORT_8042_COMD, al
	
	; Command to read from PS/2 controller output port
	call A20_WriteWaitFor8042
	mov  al, COMD_8042_READ_CONT_OUT_PORT
	out  IOPORT_8042_COMD, al
	
	; Read PS/2 controller output port
	call A20_ReadWaitFor8042
	in   al, IOPORT_8042_DATA
	push ax
	
	; Command to write to PS/2 controller output port
	call A20_WriteWaitFor8042
	mov  al, COMD_8042_WRITE_CONT_OUT_PORT
	out  IOPORT_8042_COMD, al
	
	; Enable A20 bit
	call A20_WriteWaitFor8042
	pop  ax
	or   al, A20_BIT_ENABLE
	out  IOPORT_8042_DATA, al
	
	; Enable PS/2 port 1
	call A20_WriteWaitFor8042
	mov  al, COMD_8042_ENABLE_PS2_PORT1
	out  IOPORT_8042_COMD, al
	
	; Enable PS/2 port 2
	call A20_WriteWaitFor8042                      
	mov  al, COMD_8042_ENABLE_PS2_PORT2      
	out  IOPORT_8042_COMD, al
	
	; Restore the contents of AX
	pop  ax
	ret
 

; A20_ReadWaitFor8042 : Function to poll the 8042 status register to check if we have read access to the data port 

A20_ReadWaitFor8042:
	push ax

	.readwaitloop:
	in   al, IOPORT_8042_COMD
	test al, TEST_8042_READ_ACCESS
	jz   .readwaitloop

	pop  ax
	ret

; A20_WriteWaitFor8042 : Function to poll the 8042 status register to check if we have write access to the data and command ports 

A20_WriteWaitFor8042:
	push ax

	.writewaitloop:
	in   al, IOPORT_8042_COMD
	test al, TEST_8042_WRITE_ACCESS
	jnz  .writewaitloop

	pop  ax
	ret



; A20_EnableWithFastGate : Enable the A20 line using the 'Fast A20 Gate'

A20_EnableWithFastGate:
	push ax

	in   al, 0x92
	or   al, 2
	out  0x92, al	

	pop  ax
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



