[BITS 16]


GetFloppyInfo:
	pusha                                     ; We start by pushing all the general-purpose registers onto the stack
	                                          ; This way we can restore their values after the function returns 

	; Get drive parameters so that we can read the kernel image
	push es
	mov ax, 0
	mov es, ax
	mov di, ax                                ; It is recommended we set ES:DI to 0:0 to work around some buggy BIOS

	mov ah, 8                                 ; AH=8 tells the BIOS to read the drive parameters
	mov dl, [Drive]                           ; Floppy drive A
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
	popa                                      ; Restore the original state of the general-purpose registers
	ret                                       ; Return control to the bootloader

ReadFromDisk:
	pusha                                     ; Push all the general-purpose registers onto the stack

	; Reading the 1 KB kernel loader 
	; When need to use the CHS scheme when using INT 0x13 to read from disk
	; The following relations give the conversion from LBA to CHS
	; Temp     = LBA % (heads_per_cylinder * sectors_per_track)
	; Cylinder = LBA / (heads_per_cylinder * sectors_per_track)
	; Head     = Temp / sectors_per_track
	; Sector   = Temp % sectors_per_track + 1

	; The logical block address (starting from 0) of the starting sector is stored in AX
	; The number of sectors to be read are stored in bl

	mov dx, 0                                 ; 
	div word [Sectors_Per_Cylinder]           ; Division by a word --> dividend is DX (MSB) : AX (LSB)
	                                          ; Result : Quotient --> AX ; Remainder --> DX						

	mov ch, al                                ; Cylinder

	mov ax, dx
	mov dx, 0
	div word [Sectors_Per_Track]

	add dx, 1
	mov cl, dl                                ; Sector (starts at 1)

	mov dh, al
	mov dl, byte [Drive]                      ; Drive ID in dl
	
	mov ah, 2                                 ; AH=2 tells the BIOS to read from the disk
	mov al, bl                                ; AL contains the numbers of sectors to be read out (stored in BL)
	mov bx, si                                ; INT 0x13 will load the data from the disk at ES:BX
	int 0x13                                   
	
	popa                                      ; Restore the original state of the general-purpose registers
	ret                                       ; Return control to the bootloader


ReadAndMove:
    pusha                                     ; Push all the general-purpose registers onto the stack

	mov bh, bl
	mov bl, 1
	
    .rnm:
        cmp bh, 0
        je .rnmdone
        dec bh
		push si
		call ReadFromDisk
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
		jmp .rnm

    .rnmdone:
    popa                                      ; Restore the original state of the general-purpose registers
    ret                  

Drive                db 0
Heads                db 2
Sectors_Per_Track    dw 18
Sectors_Per_Cylinder dw 18

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

