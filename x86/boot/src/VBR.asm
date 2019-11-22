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
; - The VBR saves (in addition to DL, ES:DI and DS:SI values passed down by the MBR) : 
;   * The memory address of a 16-byte partition entry (8-bytes for starting LBA and 8-bytes for ending LBA) in FS:BP
;   * A flag for whether INT 0x13 extensions are supported (least significant bit of DH is set if supported)
; - Note the VBR interface provides 64-bit LBA addresses for the partition boundaries to the next stage of the bootloader
; - This VBR only supports 32-bit LBAs (since it reads them from the MBR partition table)
; - As long as the VBR interface is maintained, some different code could also pass the partition boundaries from the GPT (16-bytes at offset 0x20 in the GPT partition entry)

; First let us include some definitions of constants that the VBR needs

SECTOR_SIZE             equ 0x0200                         ; Size of a sector (or size of the MBR, VBR)
LOAD_ADDRESS            equ 0x7C00                         ; This is where the MBR, VBR is loaded in memory
VBR_RELOC_ADDRESS       equ 0x0800                         ; This is where the MBR relocates itself to, then loads the VBR at LOAD_ADDRESS
STACK_TOP               equ 0x7C00                         ; Top of the stack used by the MBR

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

	times 128+4-($-$$)    db 0                             ; The 4 accounts for the 3 bytes taken up by the JMP instruction + 1 byte for nop

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

	Messages:
	.DiskIOErr            db 'Unable to load AVOS', 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	; This is where the VBR code starts

	VBR_Code:

	; We don't want any interrupts right now.
	
	cli
	
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
	
	mov   ebp, DWORD [si+0x08]                             ; Get the LBA (low DWORD) of the start sector of this partition from the MBR partition table [restricts us to 32-bit LBA]
	mov   ebx, DWORD [si+0x0C]                             ; Get the size of the partition in sectors from the MBR partition table [restricts us to 32-bits]

	; Initialize the segment registers to the base address values that we want to use (0x0000)
	
	xor   ax, ax
	mov   ds, ax
	mov   es, ax
	mov   fs, ax

    ; Then, we relocate the VBR code/data to memory location 0x0000:VBR_RELOC_ADDRESS

    mov   cx, SECTOR_SIZE
    mov   si, LOAD_ADDRESS
    mov   di, VBR_RELOC_ADDRESS
	cld                                                    ; Clear the direction flag so that MOVSB proceeds from low to high memory addresses
    rep   movsb
	jmp   0x0000:Start
	
	Start:

	; Lets reenable interrupts now

	sti
	
	; Add the LBA of the bootloader start sector to the Disk Address Packet (DAP) used by the extended INT 0x13 
	
	add   DWORD [DAP.Start_Sector], ebp                    ; Add the partition start sector to the sector offset of the start of the bootloader
	adc   DWORD [DAP.Start_Sector+4], 0                    ; The start sector LBA in the DAP is 64-bit, we must appropriately account for any carry after adding the above offset  
	mov   DWORD [Volume_Partition_Table], ebp              ; Save the lower 4 bytes of the 64-bit LBA of the start sector of the partition
	mov   DWORD [Volume_Partition_Table+8], ebp            ; Save the 64-bit LBA of the end sector of the partition (start + size)
	add   DWORD [Volume_Partition_Table+8], ebx
	adc   DWORD [Volume_Partition_Table+0x10], 0

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
	mov   BYTE [STACK_TOP-1], 1                            ; INT 0x13 extension exists. Lets make a note of it
	mov   dl, [STACK_TOP-2]
	mov   si, DAP
	mov   ah, 0x42
	int   0x13
	jnc   LaunchBootloader
	
	; If BIOS extensions do not exist or did not work, we will need to read from disk using INT 0x13, AH=0x02 that employs the CHS scheme
	
	DiskReadUsingCHS:
	mov   BYTE [STACK_TOP-1], 0                            ; INT 0x13 extension did not work. Lets make a note of it

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
	mov   dl, [STACK_TOP-2]
	mov   ah, 0x08
	int   0x13
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
	mov   dl, [STACK_TOP-2]                                ; Drive ID is stored in DL
	mov   al, [DAP.Sectors_Count]                          ; Number of sectors to copy are stored in AL

	; Now call INT 0x13, AH=0x02
	
	mov   ah, 0x02
	int   0x13
	jnc   LaunchBootloader
	jmp   HaltSystem
	
	; We reach here if the disk read was successful 
	; Save the DL, DS:SI and ES:DI values passed by the MBR
	; Save the INT 0x13 extensions support flat in DH
	; Save the address of the 16-byte entry containing the partition boundaries in FS:BP 	

	LaunchBootloader:
	mov   sp, STACK_TOP-10 
	pop   si
	pop   ds
	pop   di
	pop   es
	pop   dx
	mov   bp, Volume_Partition_Table
	jmp   LOAD_ADDRESS
	
	; If we did not read what we wanted to we halt
	; Before halting we print an error message on the screen 
	; We use INT 0x10 AH=0x0E to print a string to screen character-by-character
	; The character byte is stored in AL ; BH contains the display page number ; BL contains the display color for the character
	
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

; Padding of zeroes till offset 446 : location of the VBR partition table (if any)
; We put a 16-byte entry at offset 446 corresponding to the 64-bit LBAs of the start and end sectors of the partition
; The address of this 16-byte entry is passed to the next step of the bootloader in the FS:BP registers

times 446-($-$$) db 0

Volume_Partition_Table:
times 64 db 0

; Padding of zeroes till the end of the boot sector (barring the last two bytes that are reserved for the boot signature)

times 512-2-($-$$) db 0 

; The last two bytes of the boot sector need to have the following boot signature for BIOS to consider it to be valid

Boot_Signature:
dw   0xAA55

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

