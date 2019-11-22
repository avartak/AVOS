; The Volume Boot Record (VBR) is the first sector of the 'active' or bootable partition that contains our OS
; The Master Boot Record (MBR) first identifies the active partition, then loads the VBR at memory address 0x7C00, and transfers control to it
; The VBR is the first OS-specific code. 
; It reads a part (first 512 B sector) of our bootloader from disk into memory and transfers control to it 
; This VBR has been made relocatable - it relocates to a specified address (just like the MBR) and loads the first sector of the bootloader in its place 
; The VBR also has the boot signature word (0xAA55) at its end, just like the MBR
; Since the VBR is just one sector (512 bytes) long it cannot access filesystems. Therefore, it reads the bootloader from disk knowing the low-level LBA of the bootloader

; First let us include some definitions of constants that the VBR needs

SECTOR_SIZE             equ 0x0200                         ; Size of a sector (or size of the MBR, VBR)
LOAD_ADDRESS            equ 0x7C00                         ; This is where the bootloader loader is loaded in memory
STACK_TOP               equ 0x7C00                         ; Top of the stack used by the MBR
SCREEN_TEXT_BUFFER      equ 0xB800                         ; Segment address pointing to the video buffer for the 80x25 VBE text mode (for displaying error messages)
BOOTLOADER_ADDRESS      equ 0x7E00                                      ; Starting location in memory where the bootloader code gets loaded

; We need to tell the assembler that all labels need to be resolved relative to the memory address 0x7C00 in the binary code

ORG LOAD_ADDRESS

; The x86 system is still in real mode when control is transferred to the VBR
; We need to tell the assembler to produce 16-bit code

BITS 16

; This is where the VBR boot code starts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
AVBL:

	jmp   AVBL_Code
	nop

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	AVBL_BlockList:

	; We reserve the next 104 bytes for the blocklist corresponding to the bootloader code on disk
	; This is basically a table containing (up to) 10 ten-byte entries
	; The first 4 bytes of the blocklist contain the segment address, then the offset address from where to start loading the bootloader code in memory
	; Then come the blocklist entries
	; First 8 bytes of each entry contain the 64-bit LBA offset (w.r.t. the partition) of the start sector of a 'block' containing the bootloader code
	; The last 2 bytes contain the size of the block (number of contiguous sectors to be read out)
	; An entry with 0 size marks the end of the blocklist, all remaining entries will be ignored
	; Some tool provided by the OS/bootloader should fill this table. Right now it has some defaults put in

	.Load_segment         dw 0
	.Load_offset          dw BOOTLOADER_ADDRESS

	.Block1_LBA           dq 9
	.Block1_num_sectors   dw 0x40-1

	times 104+4-($-$$)    db 0                             ; The 4 accounts for the 3 bytes taken up by the JMP instruction + 1 reserved byte

	Drive_ID              db 0

	CHS_Geometry:
	.Sectors_Per_Track    db 18
	.Sectors_Per_Cylinder dw 36

	DAP:
	.Size                 db 0x10
	.Unused1              db 0
	.Sectors_Count        db 0
	.Unused2              db 0
	.Memory_Offset        dw 0
	.Memory_Segment       dw 0
	.Start_Sector         dq 0

	DiskReadFlags         db 0

	MBR_Part_Entry_Addr   dd 0

	Messages:
	.DiskIOErr            db 'Unable to load AVOS', 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	; This is where the VBR code starts

	AVBL_Code:

	; Expect the boot drive ID in DL and the active partition table entry in the relocated MBR in DS:SI when control is transferred to the boot loader. 
	; Expect DL to be 1 if INT 0x13 extensions for disk read using LBA scheme exist, and 0 if we need to resort to CHS addressing
	; ES:DI may point to "$PnP" installation check structure for systems with Plug-and-Play BIOS or BBS support
	; Save these registers on the stack

	mov   sp, STACK_TOP
	push  dx
	push  es
	push  di
	push  ds
	push  si
	push  bx
	
	mov   ebx, DWORD [si+0x08]                             ; Get the LBA of the start sector of this partition from the MBR partition table [restricts us to 32-bit LBA]

	; Lets initialize the segment registers to the base address values that we want to use (0x0000)
	
	xor   ax, ax
	mov   ds, ax
	mov   es, ax

	; Save some useful information
	
	mov   [Drive_ID], dl                                   ; Save the boot drive ID --> we expect it in DL
	mov   [MBR_Part_Entry_Addr], ebx                       ; Save the 32-bit LBA of the start sector of this partition
	mov   di, AVBL_BlockList+4                             ; Save the starting address of the bootloader blocklist entries
	pop   bx                                               ; Save the flag (passed by VBR) that indicates if extended INT 0x13 works
	and   bl, 1
	mov   [DiskReadFlags], bl
	
	; Save the CHS disk geometry in case read with LBA scheme fails

	LoadCHSGeometry:
	pusha
	xor   ax, ax
	mov   es, ax
	mov   di, ax
	mov   dl, BYTE [Drive_ID]
	mov   ah, 0x08
	int   0x13
	jnc   SaveCHSGeometry
	mov   bl, [DiskReadFlags]
	or    bl, 2
	mov   [DiskReadFlags], bl
	popa
	jmp   ReadLoop
	
	SaveCHSGeometry:
	movzx ax, dh
	inc   ax                                               
	and   cl, 0x3F                                         
	mov   [CHS_Geometry.Sectors_Per_Track], cl             
	movzx si, cl
	mul   si                                               
	mov   [CHS_Geometry.Sectors_Per_Cylinder], ax          
	popa

	ReadLoop:
	cmp   WORD [di+0x08], 0                                ; Check the number of sectors to be read out is 0 --> indicates the end of the block list
	je    LaunchBootloader

	DiskRead:
	mov   ebx, DWORD [di]
	mov   ecx, DWORD [di+0x04]

	xor   eax, eax
	mov   al, 0x7F
	cmp   [di+0x08], ax
	jg    PrepareRead
	mov   ax, [di+0x08]
	
	PrepareRead:                                           ; Save the necessary information needed by (extended) BIOS routines to read from disk --> Fill the DAP
	sub   [di+0x08], ax
	
	add   DWORD [di], eax
	adc   DWORD [di+0x04], 0

	mov   [DAP.Sectors_Count], al
	mov   [DAP.Start_Sector], ebx
	mov   [DAP.Start_Sector+0x04], ecx
	mov   ebx, [MBR_Part_Entry_Addr]
	add   DWORD [DAP.Start_Sector], ebx
	adc   DWORD [DAP.Start_Sector+0x04], 0

	mov   bx, [AVBL_BlockList]
	mov   [DAP.Memory_Segment], bx
	mov   bx, [AVBL_BlockList+0x02]
	mov   [DAP.Memory_Offset], bx

	push  di

	DiskReadUsingLBA:
	test  BYTE [DiskReadFlags], 1                          ; Check if we can read the disk using LBA scheme
	jz    DiskReadUsingCHS

	mov   dl, BYTE [Drive_ID]
	mov   si, DAP
	mov   ah, 0x42
	int   0x13
	jnc   ReadLoopCheck
	
	; If BIOS extensions do not exist or did not work, we will need to read from disk using INT 0x13, AH=0x02 that employs the CHS scheme
	
	DiskReadUsingCHS:
	mov   bl, [DiskReadFlags]
	test  bl, 2
	jnz   HaltSystem
	mov   eax, DWORD [DAP.Start_Sector+0x04]               ; Cannot read sector numbers outside the 32-bit range using CHS
	or    eax, eax
	jnz   HaltSystem

	mov   eax, DWORD [DAP.Start_Sector]
	mov   edx, 0
	movzx ebx, WORD [CHS_Geometry.Sectors_Per_Cylinder]
	div   ebx                                              ; LBA / sectors per cylinder = cylinder number
	cmp   eax, 0x3FF                                       ; cylinder number greater than 1023 is not supported by the BIOS INT 0x13, AH=0x02 routine
	jg    HaltSystem
	shl   ax, 0x6                            
	mov   cx, ax
	shl   cx, 2
	mov   cl, ah
	and   cl, 0xC0
	
	mov   ax, dx                                           ; DX contains LBA % sectors per cylinder
	div   BYTE [CHS_Geometry.Sectors_Per_Track]            ; (LBA % sectors per cylinder) % sectors per track = sector number (starting at 0)
	add   ah, 0x1                                          ; Sector number starts at 1
	and   ah, 0x3F                                         ; Sector numbers need to be in the range [1, 63]
	or    cl, ah                                           ; Store sector number in CL for INT 0x13, AH=0x02
	mov   dh, al                                           ; (LBA % sectors per cylinder) / sectors per track = head --> Store it in DH for INT 0x13, AH=0x02

	mov   es, [DAP.Memory_Segment]                         ; Store the load memory address in ES:BX
	mov   bx, [DAP.Memory_Offset]
	mov   dl, BYTE [Drive_ID]                              ; Drive ID is stored in DL
	mov   al, [DAP.Sectors_Count]                          ; Number of sectors to copy are stored in AL

	; Now call INT 0x13, AH=0x02
	
	mov   ah, 0x02
	int   0x13
	jc    HaltSystem

	; Check if we still need to loop

	ReadLoopCheck:
	pop   di
	cmp   WORD [di+0x08], 0
	jne   DiskRead
	add   di, 10
	jmp   ReadLoop
	
	; We reach here if the disk read was successful 
	
	LaunchBootloader:
	mov   sp, STACK_TOP-12
	pop   bx
	pop   si
	pop   ds
	pop   di
	pop   es
	pop   dx
	mov   ax, WORD [AVBL_BlockList]
	push  ax
	mov   ax, WORD [AVBL_BlockList+0x02]
	push  ax
	retf
	
	; If we did not read what we wanted to we halt
	; Before halting we print an error message on the screen 
	; In the PC architecture the video buffer sits at 0xB8000
	; We put the ES segment at that location
	; We can write to the video buffer as if it is array of characters (in fact it's an array of the character byte + attribute byte)
	; the usual VGA compatible text mode has 80 columns (i.e. 80 characters per line) and 25 rows (i.e. 25 lines)
	; We will print the error message on the penultimate line, in red
	
	HaltSystem:
	mov   si, Messages.DiskIOErr
	.printchar:
	lodsb
	test  al, al
	jz    .printdone
	mov   ah, 0x0E
	mov   bx, 0x0007
	int   0x10
	jmp   .printchar
	
	.printdone:
	cli
	hlt
	jmp   .printdone

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Padding of zeroes till the end of the boot sector (barring the last two bytes that are reserved for the boot signature)

times 512-($-$$) db 0 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


