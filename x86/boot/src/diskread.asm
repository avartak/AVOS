BITS 16

GetDiskGeometry:


	push bp
	mov  bp, sp	
	
	mov  di, 0
	mov  dl, BYTE [bp+0x4]
	mov  ah, 0x08
	int  0x13
	jc   .retfalse
	
	add  dh, 0x1
	mov  [Heads], dh
	cmp  cl, 0x3F
	jg   .retfalse
	and  cl, 0x3F
	mov  [Sectors_Per_Track], cl
	mov  al, cl
	mul  dh
	mov  [Sectors_Per_Cylinder], ax
	
	.rettrue:
	mov  al, 1
	jmp  .end
	
	.retfalse:
	mov  al, 0
	
	.end:
	mov  sp, bp
	pop  bp
	
	ret


; CheckForBIOSExtensions : Check to see if BIOS supports extensions that allow disk I/O using LBA
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

CheckForBIOSExtensions:

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
	

; ReadFromDisk : Read from disk using BIOS extensions that use the LBA scheme (preferably) or with the 'old' BIOS routine that exploys the CHS scheme
; The routine takes five input parameters 
; (1) --> Starting sector (32-bit) 
; (2) --> Number of consecutive sectors to be read out (32-bit)
; (3) --> Destination memory address
; (4) --> Area in low memory where disk data will be copied sector-by-sector before moving it to upper memory (16-bit)
; (5) --> Drive ID (8-bit)
; The routine itself relies on INT 0x13, AH=0x42 BIOS routine
; The BIOS routine itself takes the following inputs
; AH = 0x41 (Duh!)
; DL = Drive index
; SI = Address of the DAP (disk address packet)

ReadFromDisk:

	push bp
	mov  bp, sp
	
	movzx dx, BYTE [bp+0x12]
	push dx
	call CheckForBIOSExtensions
	test al, al
	jnz  DiskReadUsingLBA
	
	call GetDiskGeometry
	test al, al
	jz   .retfalse	
	jmp  DiskReadUsingCHS
	
	.rettrue:
	mov al, 1
	jmp .end
	
	.retfalse:
	mov al, 0
	
	.end:
	mov sp, bp
	pop bp
	ret	
	
	DiskReadUsingLBA:
	
		mov eax, DWORD [bp+0x4]
		mov ebx, DWORD [bp+0x8]
		mov edi, DWORD [bp+0xC]
		mov  si,  WORD [bp+0x10]
		mov  dl,  BYTE [bp+0x12]
		
		mov DWORD [DAP_Start_Sector] , eax
		mov  WORD [DAP_Memory_Offset], si
		
		.readnmove:
		    test ebx, ebx
		    jz   ReadFromDisk.rettrue
		
		    mov  si, Disk_Address_Packet
		    mov  ah, 0x42
		    int  0x13
		    jc   ReadFromDisk.rettrue                          ; This is a hack. Need to put the exact number of sectors for the kernel image in EBX, and then if carry is set then halt system
		
		    movzx esi, WORD [DAP_Memory_Offset]
		    mov  ecx, 0x200
		    a32  rep movsb
		
		    dec  ebx
		    inc  DWORD [DAP_Start_Sector]
		    jmp  .readnmove
		
	DiskReadUsingCHS:
	
		mov  eax, DWORD [bp+0x4]
		mov  ebx, DWORD [bp+0x8]
		mov  edi, DWORD [bp+0xC]
		
		mov  DWORD [CHS_Start_Sector] , eax
		mov  DWORD [CHS_Sectors_Count], ebx
		mov  DWORD [CHS_Memory_Offset], edi
	
		.readnmove:
			mov  ebx, DWORD [CHS_Sectors_Count]
		    test ebx, ebx
		    jz   ReadFromDisk.rettrue
		
			mov  eax, DWORD [CHS_Start_Sector]
			mov  edx, 0
			movzx ebx, WORD [Sectors_Per_Cylinder]
			div  ebx
			cmp  eax, 0x3FF
			jg   ReadFromDisk.retfalse
			shl  ax, 0x6
			mov  cx, ax
		
			mov  ax, dx
			div  BYTE [Sectors_Per_Track]
			add  ah, 0x1
			and  ah, 0x3F
			or   cl, ah
		
			mov  dh, al
		
			mov  bx, WORD [bp+0x10]
			mov  dl, BYTE [bp+0x12]
			mov  al, 1
		
			mov  ah, 0x02
			int  0x13
		    jc   ReadFromDisk.rettrue                          ; This is a hack. Need to put the exact number of sectors for the kernel image in EBX, and then if carry is set then halt system
		
			movzx esi, WORD [bp+0x10]
			mov  edi, DWORD [CHS_Memory_Offset]
		    mov  ecx, 0x200
		    a32  rep movsb
			mov  DWORD [CHS_Memory_Offset], edi
		
			dec  DWORD [CHS_Sectors_Count]
			inc  WORD [CHS_Start_Sector]
			jmp  .readnmove
		

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Heads                db 0
Sectors_Per_Track    db 0
Sectors_Per_Cylinder dw 0

CHS_Start_Sector     dd 0
CHS_Sectors_Count    dd 0
CHS_Memory_Offset    dd 0

Disk_Address_Packet:
DAP_Size             db 0x10
DAP_Unused1          db 0
DAP_Sectors_Count    db 1
DAP_Unused2          db 0
DAP_Memory_Offset    dw 0
DAP_Memory_Segment   dw 0
DAP_Start_Sector     dq 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

