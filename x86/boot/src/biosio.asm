BITS 16


; ReadDriveParameters : Get drive parameters which we will use to read from the drive
; Drive ID is stored in DL 

ReadDriveParameters:
	pusha                                     ; We start by pushing all the general-purpose registers onto the stack

	push es
	mov ax, 0
	mov es, ax
	mov di, ax                                ; It is recommended we set ES:DI to 0:0 to work around some buggy BIOS

	mov ah, 8                                 ; AH=8 tells the BIOS to read the drive parameters ; Drive ID is stored in DL
	int 0x13                                  ; INT 0x13 is all about I/O from disks

	add dh, 1                                 ; Number of heads is stored in DH (numbering starts at 0, hence the increment)
	mov [Heads], dh                           ; Store this information in a variable

	and cx, 0x3F                              ; Bits 0-5 of CX store the number of sectors per track
	mov [Sectors_Per_Track], cx               ; Store this information in a variable
	mov ax, cx
	mov bh, [Heads]
	mul bh
	mov [Sectors_Per_Cylinder], ax

	pop es                                    ; Restore the ES register to its original state
	popa                                      ; Restore the all the general-purpose registers

	mov [Return_Code_Last], ah                ; Lets also store the return code of the last operation

	ret                                       ; Return control to the bootloader



; ReadSectorsFromDrive: Read a certain number of sectors from a drive
; Drive ID is stored in DL
; First sector to read out is stored in AX
; Number of sectors to read out is stored in BL
; Data from the disk is copied to memory starting at ES:SI

; When need to use the CHS scheme when using INT 0x13 to read from disk
; The following relations give the conversion from LBA to CHS
; Temp     = LBA % (heads_per_cylinder * sectors_per_track)
; Cylinder = LBA / (heads_per_cylinder * sectors_per_track)
; Head     = Temp / sectors_per_track
; Sector   = Temp % sectors_per_track + 1

ReadSectorsFromDrive:
	pusha                                     ; Push all the general-purpose registers onto the stack

	mov [Drive], dl                           ; Store the Drive ID (to free up DL)

	mov dx, 0                                 ; 
	div word [Sectors_Per_Cylinder]           ; Division by a word --> dividend is DX (MSB) : AX (LSB)
	                                          ; Result : Quotient --> AX ; Remainder --> DX						

	mov ch, al                                ; Cylinder

	mov ax, dx
	mov dx, 0
	div word [Sectors_Per_Track]

	add dx, 1
	mov cl, dl                                ; Sector (starts at 1)

	mov dh, al                                ; Head

	mov dl, [Drive]                           ; Restore the Drive ID into DL
	
	mov ah, 2                                 ; AH=2 tells the BIOS to read from the disk ; Drive ID is stored in DL
	mov al, bl                                ; AL contains the numbers of sectors to be read out (stored in BL)
	mov bx, si                                ; INT 0x13 will load the data from the disk at ES:BX
	int 0x13                                   

	mov [Sectors_Read_Last], al               ; Lets store the number of sectors that were actually read out
	mov [Return_Code_Last], ah                ; Lets also store the return code of the last operation

	popa                                      ; Restore the original state of the general-purpose registers

	add ax, [Sectors_Read_Last]               ; We will increment AX with the number of sectors that were read out
	                                          ; Currently there is no exception handling if some of the sectors were not read. This is not safe

	ret                                       ; Return control to the bootloader



; ReadAndMove : Read a certain number of sectors from the drive and move them to a high address space (>1 MB)
ReadAndMove:
	push bx                                   ; Only push the registers that are affected
	push cx
	push dx

	mov bh, bl
	mov bl, 1
	
    .rnm:
        cmp bh, 0
        je .rnmdone

		call ReadSectorsFromDrive             ; There is a problem here -- what is we don't read that sector ? A proper exception handling is needed. This code is not safe
		mov cx, [Sectors_Read_Last]           ; Check if a sector was actually read and only then copy ES:SI to ES:EDI
		cmp cx, 0
		je .rnm                               ; Keep trying to read the sector if the read failed

		push si
		mov cx, 512
        .rnmloop:
            mov dh, [es:si]
            mov [es:edi], dh
            inc si
			inc edi
			loop .rnmloop
		pop si

        dec bh
		jmp .rnm

    .rnmdone:
    pop dx                                    ; Restore the original state of the affected registers
    pop cx                                    
    pop bx                                    
    ret                                       ; Note that EDI is incremented by the number of bytes moved

Drive                db 0
Heads                db 2
Sectors_Per_Track    dw 18
Sectors_Per_Cylinder dw 18

Sectors_Read_Last    db 0
Return_Code_Last     db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

