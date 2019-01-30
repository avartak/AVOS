BITS 16


; ReadDriveParameters : Get drive parameters which we will use to read from the drive
; Drive ID is stored in DL 

ReadDriveParameters:

	push bp
	mov bp, sp	

	pusha                                     ; We start by pushing all the general-purpose registers onto the stack
	push es

	mov dl, [Drive]                           ; Drive ID

	mov ax, 0
	mov es, ax
	mov di, ax                                ; It is recommended we set ES:DI to 0:0 to work around some buggy BIOS

	mov ah, 8                                 ; AH=8 tells the BIOS to read the drive parameters ; Drive ID is stored in DL
	int 0x13                                  ; INT 0x13 is all about I/O from disks

	mov [Return_Code_Last], ah                ; Store the return code of the BIOS function

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

	mov sp, bp
	pop bp

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

	push bp
	mov bp, sp	

	pusha                                     ; Push all the general-purpose registers onto the stack

    mov  ax, [bp+0x4]                         ; Starting sector
    mov  bx, [bp+0x6]                         ; Number of sectors to copy
    mov  si, [bp+0x8]                         ; Where to load the sectors in memory

	                                          ; We are moving 1 MB of data from the disk as kernel
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

	mov dl, [Drive]                           ; Store the Drive ID into DL
	
	mov al, bl                                ; AL contains the numbers of sectors to be read out (stored in BL)
	mov bx, si                                ; INT 0x13 will load the data from the disk at ES:BX

	mov ah, 2                                 ; AH=2 tells the BIOS to read from the disk ; Drive ID is stored in DL
	int 0x13                                   

	mov [Sectors_Read_Last], al               ; Lets store the number of sectors that were actually read out
	mov [Return_Code_Last], ah                ; Lets also store the return code of the last operation

	popa                                      ; Restore the original state of the general-purpose registers

	add ax, [Sectors_Read_Last]               ; We will increment AX with the number of sectors that were read out
	                                          ; Currently there is no exception handling if some of the sectors were not read. This is not safe

	mov sp, bp
	pop bp

	ret                                       ; Return control to the bootloader



; ReadAndMove : Read a certain number of sectors from the drive and move them to a high address space (>1 MB)
ReadAndMove:

	push bp
	mov bp, sp	

	push bx                                   ; Only push the registers that are affected
	push cx
	push dx

    mov  ax, [bp+0x4]                         ; Starting sector
    mov  bx, [bp+0x6]                         ; Number of sectors to copy
    mov  si, [bp+0x8]                         ; Temporary location to copy sectors before moving them to high memory
    mov edi, [bp+0xA]                         ; High memory destination

	mov bh, bl
	mov bl, 1
	
    .rnm:
        cmp bh, 0
        je .rnmdone

		push si
		push bx
		push ax
	
		call ReadSectorsFromDrive
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

		add sp, 0x6
        dec bh
		jmp .rnm

    .rnmdone:
    pop dx                                    ; Restore the original state of the affected registers
    pop cx                                    
    pop bx                                    

	mov sp, bp
	pop bp

    ret                                       ; Note that EDI is incremented by the number of bytes moved

Drive                db 0x80
Heads                db 2
Sectors_Per_Track    dw 18
Sectors_Per_Cylinder dw 18

Sectors_Read_Last    db 0
Return_Code_Last     db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

