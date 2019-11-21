; The Volume Boot Record (VBR) is the first sector of the 'active' or bootable partition that contains our OS
; The Master Boot Record (MBR) first identifies the active partition, then loads the VBR at memory address 0x7C00, and transfers control to it
; The VBR is the first OS-specific code
; Salient features of our VBR :
; - Starts with JMP to the VBR code
; - There are 128 bytes left for OS-specific (or filesystem specific) data/header after the first three bytes of the JMP instruction (the JMP jumps over this block) 
; - The VBR also has the boot signature word (0xAA55) at its end, just like the MBR
; - The VBR first relocates itself to another location in memory 
; - The VBR then loads exactly 1 sector (512 B) of bootloader code that bootstraps the loading of the entire bootloader from disk to memory
; - This keeps the VBR simple : it only needs to know the sector number (or even just the sector offset from the start of the partition) of the sector to load

; First let us include some definitions of constants that the VBR needs

SECTOR_SIZE             equ 0x0200                         ; Size of a sector (or size of the MBR, VBR)
LOAD_ADDRESS            equ 0x7C00                         ; This is where the MBR, VBR is loaded in memory
VBR_RELOC_ADDRESS       equ 0x0800                         ; This is where the MBR relocates itself to, then loads the VBR at LOAD_ADDRESS
STACK_TOP               equ 0x7C00                         ; Top of the stack used by the MBR
SCREEN_TEXT_BUFFER      equ 0xB800                         ; Segment address pointing to the video buffer for the 80x25 VBE text mode (for displaying error messages)

BOOTLOADLD_PART_START   equ 8                              ; Starting sector of the boot loader on the partition

; We need to tell the assembler that all labels need to be resolved relative to the memory address 0x7C00 in the binary code

ORG VBR_RELOC_ADDRESS

; The x86 system is still in real mode when control is transferred to the VBR
; We need to tell the assembler to produce 16-bit code

BITS 16

; This is where the VBR boot code starts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VBR:

	jmp   VBR_Code
	nop

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	; We reserve the next 128 bytes of the VBR for any filesystem related data structure (e.g. the BIOS Parameter Block in FAT file systems)

	times 128+4-($-$$)    db 0                             ; The 3 accounts for the 3 bytes taken up by the JMP instruction

	Drive_ID              db 0

	DAP:                                                   ; We put the DAP at a known offset of 132 bytes from the start of the VBR
	.Size                 db 0x10                          ; The start sector (relative to the partition) of the bootloader loader can be modified when installing the OS
	.Unused1              db 0
	.Sectors_Count        db 1                             ; The bootloader loader is exactly one sector in size
	.Unused2              db 0
	.Memory_Offset        dw LOAD_ADDRESS                  ; Where to put the bootloader loader in memory
	.Memory_Segment       dw 0
	.Start_Sector         dq BOOTLOADLD_PART_START

	CHS_Geometry:                                          ; We put the CHS geometry (and a flag byte indicating of CHS access is needed) at an offset of 148 bytes 
	.Sectors_Per_Track    db 18                            ; Software downstream can directly import this information if need be
	.Sectors_Per_Cylinder dw 36                            ; The VBR is relocated to VBR_RELOC_ADDRESS --> Downstream software will need to know this to be able to access CHS geometry

	DiskReadFlags         db 0

	Messages:
	.DiskIOErr            db 'Unable to load AVOS', 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	; This is where the VBR code starts

	VBR_Code:

	; We don't want any interrupts right now.
	
	cli
	
	; Clear the direction flag so that the string operations (that we will use later) proceed from low to high memory addresses

	cld

	; We first set up a usable stack

	xor   ax, ax
	mov   ss, ax
	mov   sp, STACK_TOP 

	; BIOS stores the boot drive ID in DL and the active partition table entry in the relocated MBR in DS:SI when control is transferred to the boot loader. 
	; Save these registers on the stack
	; Also save the ES and DI registers. ES:DI points to "$PnP" installation check structure for systems with Plug-and-Play BIOS or BBS support

	push  dx
	push  es
	push  di
	push  ds
	push  si
	
	mov   ebx, DWORD [si+0x08]                             ; Get the LBA (low DWORD) of the start sector of this partition from the MBR partition table [restricts us to 32-bit LBA]

	; The MBR is expected to make a jump to 0x0000:0x7C00 (as opposed to 0x07C0:0x0000 or something else) but lets not assume this
	; We should initialize the segment registers to the base address values that we want to use (0x0000)
	
	xor   ax, ax
	mov   ds, ax
	mov   es, ax

	; Lets reenable interrupts now

	sti
	
    ; Then, we relocate the VBR code/data to memory location 0x0000:VBR_RELOC_ADDRESS

    mov   cx, SECTOR_SIZE
    mov   si, LOAD_ADDRESS
    mov   di, VBR_RELOC_ADDRESS
    rep   movsb
	jmp   0x0000:Start
	
	Start:

	; Set video to 80x25 text mode

	mov   ax, 0x0003
	int   0x10

	; Save the LBA of the bootloader start sector in the Disk Address Packet (DAP) used by the extended INT 0x13 
	
	mov   [Drive_ID], dl                                   ; Save the boot drive ID --> MBR put it in DL
	add   DWORD [DAP.Start_Sector], ebx                    ; Add the partition start sector sector to the sector offset of the start of the bootloader
	adc   DWORD [DAP.Start_Sector+4], 0                    ; The start sector LBA in the DAP is 64-bit, we must appropriately account for any carry after adding the above offset  

	; Check for BIOS extensions to read from disk using the LBA scheme

	mov   bx, 0x55AA
	mov   ah, 0x41
	int   0x13
	jc    DiskReadUsingCHS
	cmp   bx, 0xAA55
	jne   DiskReadUsingCHS
	test  cx, 1
	jz    DiskReadUsingCHS
	
	; BIOS extensions exist. So, we use the INT 0x13, AH=0x42 BIOS routine to read from disk (See MBR code for more details)
	
	DiskReadUsingLBA:
	mov   BYTE [DiskReadFlags], 1                          ; INT 0x13 extension exists. Lets make a note of it
	mov   dl, BYTE [Drive_ID]
	mov   si, DAP
	mov   ah, 0x42
	int   0x13
	jnc   LaunchBootloader
	
	; If BIOS extensions do not exist or did not work, we will need to read from disk using INT 0x13, AH=0x02 that employs the CHS scheme
	
	DiskReadUsingCHS:
	mov   BYTE [DiskReadFlags], 0                          ; INT 0x13 extension did not work. Lets make a note of it

	; First we read the disk geometry using INT 0x13, AH=0x08
	; - It is recommended to set ES:DI to 0x0000:0x0000 to work around some buggy BIOS
	; - DL should contain the drive ID
	; - AH=0x08
	; Returns:
	; - CF set on error
	; - AH contains error code if CF is set
	; - DL contains number of disk drives
	; - BL contains drive type (only AT/PS2 floppies) 
	; - ES:DI contains pointer to drive parameter table (only for floppies) 
	; - DH contains number of heads - 1
	; - CL bits 0:5 --> sectors per track
	; - CH and then CL bits 6:7 (as most significant bits) --> number of cylinders 

	xor   ax, ax
	mov   es, ax
	mov   di, ax
	mov   dl, BYTE [Drive_ID]
	mov   ah, 0x08
	int   0x13
	mov   si, Messages.DiskIOErr   
	jc    HaltSystem

	movzx ax, dh
	inc   ax                                               ; Since DH contains number of heads - 1, we add one to get the number of heads (technically can be 256, hence store in a word)
	and   cl, 0x3F                                         ; First 6 bits of CL contain the number of sectors per track
	mov   [CHS_Geometry.Sectors_Per_Track], cl             ; Save the number of sectors per track to memory
	movzx si, cl
	mul   si                                               ; number of sectors per cylinder = number of sectors per track x number of heads
	mov   [CHS_Geometry.Sectors_Per_Cylinder], ax          ; Save the number of sectors per cylinder in memory
	
	; This is the code that converts the address of the starting sector from the LBA scheme to the CHS scheme 
	; It stores the C,H,S values in appropriate registers and then calls INT 0x13, AH=0x02
	; More details on the INT 0x13, AH=0x02 BIOS routine can be found in the MBR code

	mov   eax, DWORD [DAP.Start_Sector+4]
	or    eax, eax                                         ; Test for a 32-bit LBA (that's the most that can be supported with CHS)
	jnz   HaltSystem                                       
	mov   eax, DWORD [DAP.Start_Sector]
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

	xor   ax, ax
	mov   es, ax
	mov   bx, LOAD_ADDRESS                                 ; Store the load memory address in ES:BX
	mov   dl, BYTE [Drive_ID]                              ; Drive ID is stored in DL
	mov   al, BYTE [DAP.Sectors_Count]                     ; Number of sectors to copy are stored in AL

	; Now call INT 0x13, AH=0x02
	
	mov   ah, 0x02
	int   0x13
	jnc   LaunchBootloader
	jmp   HaltSystem
	
	; We reach here if the disk read was successful 
	
	LaunchBootloader:
	movzx bx, BYTE [DiskReadFlags]                         ; Store the flag that indicates extended INT 0x13 support in BX 
	mov   sp, STACK_TOP-10 
	pop   si
	pop   ds
	pop   di
	pop   es
	pop   dx
	jmp   LOAD_ADDRESS
	
	; If we did not read what we wanted to we halt
	; Before halting we print an error message on the screen 
	; In the PC architecture the video buffer sits at 0xB8000
	; We put the ES segment at that location
	; We can write to the video buffer as if it is array of characters (in fact it's an array of the character byte + attribute byte)
	; the usual VGA compatible text mode has 80 columns (i.e. 80 characters per line) and 25 rows (i.e. 25 lines)
	; We will print the error message on the penultimate line, in red
	
	HaltSystem:
	mov   ax, SCREEN_TEXT_BUFFER
	mov   es, ax             
	mov   di, 80*23*2        
	mov   si, Messages.DiskIOErr   
	.printchar:
		lodsb                
		test  al, al        
		jz    .printdone    
		mov   ah, 0x04      
		stosw                   
		jmp   .printchar 
	.printdone:	
	cli
	hlt
	jmp   .printdone

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Padding of zeroes till offset 440 (location of the MBR signature and then the partition table). We will keep this space empty

times 440-($-$$) db 0

VBR_Signature:
times  6 db 0

Volume_Partition_Table:
times 64 db 0

; Padding of zeroes till the end of the boot sector (barring the last two bytes that are reserved for the boot signature)

times 512-2-($-$$) db 0 

; The last two bytes of the boot sector need to have the following boot signature for BIOS to consider it to be valid

Boot_Signature:
dw   0xAA55

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

