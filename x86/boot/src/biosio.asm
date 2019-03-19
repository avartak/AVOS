BITS 16

; BIOS_CheckForExtensions : Check to see if BIOS supports extensions that allow disk I/O using LBA
; In order to check if BIOS supports extensions  we need to call INT 0x13, AH=0x41
; It's input parameters are as follows
; AH = 0x41 (Duh!)
; DL = Drive index
; BX = 0x55AA
; The output parameters are as follows
; CF = Set if not present, clear if present
; AH = Error code or major version
; BX = 0xAA55
; CX = Bit 1 : Device Access using the packet structure ; Bit 2 : Drive Locking and Ejecting ; Bit 3 : Enhanced Disk Drive Support (EDD) ; We are only concerned with the first bit being set 
; Output is stored as a boolean (0 or 1) in AL

BIOS_CheckForExtensions:

	push bp
	mov  bp, sp

	mov  dl, BYTE [bp+0x4]

	mov  ah, 0x41
	mov  bx, 0x55AA
	int  0x13
	jc   .retfalse
	cmp  bx, 0xAA55
	jne  .retfalse
	test cx, 1
	jz   .retfalse
	
	.rettrue:
	mov al, 1
	jmp .end
	
	.retfalse:
	mov al, 0
	
	.end:
	mov sp, bp
	pop bp
	ret
	

; BIOS_ReadFromDiskToLowMemory : Read from disk using BIOS extensions
; The routine takes two input parameters 
; (1) --> Starting sector 
; (2) --> Number of consecutive sectors to be read out
; The routine itself relies on INT 0x13, AH=0x42 BIOS routine
; The BIOS routine itself takes the following inputs
; AH = 0x41 (Duh!)
; DL = Drive index
; SI = Address of the DAP (disk address packet)

BIOS_ReadFromDiskToHighMemory:

	push bp
	mov  bp, sp

	mov eax, DWORD [bp+0x4]
	mov ebx, DWORD [bp+0x8]
	mov edi, DWORD [bp+0xC]
	mov  si,  WORD [bp+0x10]
	mov  dl,  BYTE [bp+0x12]

	mov DWORD [DAP_Start_Sector] , eax
	mov  BYTE [DAP_Sectors_Count], 1
	mov  WORD [DAP_Memory_Offset], si

    .readnmove:
        test ebx, ebx
        jz   .rettrue

        mov si, Disk_Address_Packet
        mov ah, 0x42
        int 0x13
        jc  .rettrue                          ; This is a hack. Need to put the exact number of sectors for the kernel image in EBX, and then if carry is set then halt system

        movzx esi, WORD [DAP_Memory_Offset]
        mov ecx, 0x200
        a32 rep movsb

        dec ebx
        inc DWORD [DAP_Start_Sector]
        jmp .readnmove


	mov ah, 0x42
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

Disk_Address_Packet:
DAP_Size             db 0x10
DAP_Unused1          db 0
DAP_Sectors_Count    db 0
DAP_Unused2          db 0
DAP_Memory_Offset    dw 0
DAP_Memory_Segment   dw 0
DAP_Start_Sector     dq 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

