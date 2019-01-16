BITS 16


; ReadDriveParameters : Get drive parameters which we will use to read from the drive
; Drive ID is stored in DL 

ReadDriveParameters:
	pusha                                     ; We start by pushing all the general-purpose registers onto the stack
	pushf                                     ; Push the FLAGS register as well -- why not

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
	popf                                      ; Restore the FLAGS register
	popa                                      ; Restore the all the general-purpose registers
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
	pushf                                     ; Push the FLAGS register as well -- why not
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
	
	popf                                      ; Restore the FLAGS register
	popa                                      ; Restore the original state of the general-purpose registers
	ret                                       ; Return control to the bootloader



; ReadAndMove : Read a certain number of sectors from the drive and move them to a high address space (>1 MB)
ReadAndMove:
    pusha                                     ; Push all the general-purpose registers onto the stack
	pushf                                     ; Push the FLAGS register as well -- why not

	mov bh, bl
	mov bl, 1
	
    .rnm:
        cmp bh, 0
        je .rnmdone
        dec bh
		push si
		call ReadSectorsFromDrive
        mov ecx, 512
        .rnmloop:
            mov dl, [es:si]
            mov [es:edi], dl
            inc si
			inc edi
            dec ecx
            cmp ecx, 0
            jne .rnmloop
		pop si
		inc ax
		jmp .rnm

    .rnmdone:
	popf                                      ; Restore the FLAGS register
    popa                                      ; Restore the original state of the general-purpose registers
    ret                  

Drive                db 0
Heads                db 2
Sectors_Per_Track    dw 18
Sectors_Per_Cylinder dw 18

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

