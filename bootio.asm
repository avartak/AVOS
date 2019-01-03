[BITS 16]

GetDriveInfo:
	pusha                                     ; We start by pushing all the general-purpose registers onto the stack
	                                          ; This way we can restore their values after the function returns 

	; Get drive parameters so that we can read the kernel image
	push es
	mov ax, 0
	mov es, ax
	mov di, ax                                ; It is recommended we set ES:DI to 0:0 to work around some buggy BIOS
	mov ah, 8                                 ; AH=8 tells the BIOS to read the drive parameters
	int 0x13                                  ; INT 0x13 is all about I/O from disks
	and cx, 0x3F                              ; Bits 0-5 of CX store the number of sectors per track
	mov [Sectors_Per_Track], cx               ; Store this information in a variable
	add dh, 1                                 ; Number of heads is stored in DH (numbering starts at 0, hence the increment)
	mov [Sides], dh                           ; Store this information in a variable

	pop es                                    ; Restore the ES register to its original state
	popa                                      ; Restore the original state of the general-purpose registers
	ret                                       ; Return control to the bootloader

; The LoadKernel function - load kernel image from disk to memory (ES:SI to be specific)
; We are missing some functionality here
; What if we encounter problems reading from the disk ? There should be some protection for that
; Also, who uses floppy disks anymore ? :) But ok, baby steps ...

LoadKernel:
	pusha                                     ; Push all the general-purpose registers onto the stack

	; Reading the kernel image from disk (The kernel is just one sector long for now)
	; When need to use the CHS scheme when using INT 0x13 to read from disk
	; The following relations give the conversion from LBA to CHS
	; Temp     = LBA / (Sectors per Track) 
	; Sector   = (LBA % (Sectors per Track)) + 1
	; Head     = Temp % (Number of Heads)
	; Cylinder = Temp / (Number of Heads) 

	mov ax, 1                                 ; Logical block address (LBA) of the sector to be read - starts from 0
	mov dx, 0                                 ; 
	div word [Sectors_Per_Track]              ; Division by a word --> dividend is DX (MSB) : AX (LSB)
	                                          ; Result : Quotient --> AX ; Remainder --> DX						
	add dx, 1                                 ; Sectors start at 1, hence we need to increment the remainder 
	mov cl, dl                                ; Sector number on the track should be stored in CL
	mov dx, 0                                 
	div word [Sides]                          ; Divide the quotient (number of tracks) by the number of heads
	mov ch, al                                ; The quotient is the cylinder which goes in CH
	mov dh, dl                                ; The remainder is the head which should go in DH
	
	mov dl, byte [bootdev]                    ; Drive ID in dl
	
	mov bx, si                                ; INT 0x13 will load the data from the disk at ES:BX
	mov ah, 2                                 ; AH=2 tells the BIOS to read from the disk
	mov al, 2                                 ; AL contains the numbers of sectors to be read out
	int 0x13                                   
	
	popa                                      ; Restore the original state of the general-purpose registers
	ret                                       ; Return control to the bootloader


Sectors_Per_Track dw 18
Sides             db 2
bootdev           db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

