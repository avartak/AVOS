BITS 16

; BIOS_ReadDiskParameters : Get drive parameters which we will use to read from the drive
; Drive ID is stored in DL 

BIOS_ReadDiskParameters:

	mov dl, [Drive]                           ; Drive ID
	mov di, 0                                 ; It is recommended we set ES:DI to 0:0 to work around some buggy BIOS ; ES has already been set to 0
	mov ah, 8                                 ; AH=8 tells the BIOS to read the drive parameters ; Drive ID is stored in DL
	int 0x13                                  ; INT 0x13 is all about I/O from disks

	mov [Return_Code_Last], ah                ; Store the return code of the BIOS function

	add dh, 1                                 ; Number of heads is stored in DH (numbering starts at 0, hence the increment)
	mov [Heads], dh                           ; Store this information in memory

	and cx, 0x3F                              ; Bits 0-5 of CX store the number of sectors per track
	mov [Sectors_Per_Track], cx               ; Store this information in memory

	mov al, cl
	mov bh, [Heads]
	mul bh
	mov [Sectors_Per_Cylinder], ax

	ret                                       ; Return control to the bootloader



; BIOS_ReadFromDiskToLowMemory: Read a certain number of sectors from a drive
; Call the INT 0x13, AH=2 BIOS routine that requires : 
; - Drive ID to be stored in DL
; - Starting sector for read out to be stored in AX
; - Number of sectors to read out to be stored in BL
; Data from the disk is copied to memory starting at ES:SI

; The following relations give the conversion from LBA to CHS
; Temp     = LBA % (heads_per_cylinder * sectors_per_track)
; Cylinder = LBA / (heads_per_cylinder * sectors_per_track)
; Head     = Temp / sectors_per_track
; Sector   = Temp % sectors_per_track + 1

BIOS_ReadFromDiskToLowMemory:

	push bp
	mov bp, sp	

	pusha

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

	popa

	add ax, [Sectors_Read_Last]               ; We will increment AX with the number of sectors that were read out
	                                          ; Currently there is no exception handling if some of the sectors were not read. This is not safe
	mov sp, bp
	pop bp

	ret                                       ; Return control to the bootloader



; BIOS_ReadFromDiskToHighMemory : Read a certain number of sectors from the drive and move them to a high address space (>1 MB)
BIOS_ReadFromDiskToHighMemory:

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
	
		call BIOS_ReadFromDiskToLowMemory
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



; BIOS_CheckForExtensions : Check to see if BIOS supports extensions that allow disk I/O using LBA
; In order to check if BIOS supports extensions  we need to call INT 0x13, AH=0x41
; It's input parameters are as follows
; AH = 0x41 (Duh!)
; DL = Drive index
; BX = 0x55AA
; The output parameters are as follows
; CF = Set on not present, clear if present
; AH = Error code or major version
; BX = 0xAA55
; CX = Bit 1 : Device Access using the packet structure ; Bit 2 : Drive Locking and Ejecting ; Bit 3 : Enhanced Disk Drive Support (EDD) ; We are only concerned with the first bit being set 
; Output is stored as a boolean (0 or 1) in AL

BIOS_CheckForExtensions:

	push bp
	mov bp, sp	

	mov dl, [Drive]

    mov ah, 0x41
    mov bx, 0x55AA
    int 0x13
    jc  .retfalse
    cmp bx, 0xAA55
    jne .retfalse
    test cx, 1
    jz  .retfalse

	.rettrue:
	mov al, 1
	jmp .end

	.retfalse:
	mov al, 0

	.end:
	mov sp, bp
	pop bp

	ret


; BIOS_ExtReadFromDiskToLowMemory : Read from disk using BIOS extensions
; The routine takes two input parameters 
; (1) --> Starting sector 
; (2) --> Number of consecutive sectors to be read out
; The routine itself relies on INT 0x13, AH=0x42 BIOS routine
; The BIOS routine itself takes the following inputs
; AH = 0x41 (Duh!)
; DL = Drive index
; SI = Address of the DAP (disk address packet)

BIOS_ExtReadFromDisk:

	push bp
	mov bp, sp	

	mov ax, [bp+0x4]
	mov [DAP_Start_Offset], ax
	mov ax, [bp+0x6]
	mov [DAP_Sectors_Count], ax

    mov ah, 0x42
    mov dl, [Drive]
    mov si, Disk_Address_Packet
    int 0x13
    jc .retfalse

	.rettrue:
	mov al, 1
	jmp .end

	.retfalse:
	mov al, 0

	.end:
	mov sp, bp
	pop bp

	ret


Drive                db 0x80
Heads                db 2
Sectors_Per_Track    dw 18
Sectors_Per_Cylinder dw 36

Sectors_Read_Last    db 0
Return_Code_Last     db 0

Disk_Address_Packet:
DAP_Size             db 0x10
DAP_Unused           db 0
DAP_Sectors_Count    dw 0
DAP_Start_Offset     dw 0
DAP_Start_Segment    dw 0
DAP_Start_Sector     dq 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

