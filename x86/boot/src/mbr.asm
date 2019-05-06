; This file contains the boot loader code in the master boot record (MBR) of a bootable drive
; MBR is the first sector (exactly 512 B) of the boot drive
; The last word of this sector has to be 0xAA55 -- this is the boot signature
; The BIOS will look for this signature, then load this sector at memory location 0x7C00, and jump to 0x0000:0x7C00 (or sometimes 0x07C0:0x0000 -- beware!)
; The boot drive ID is stored in DL 

; The MBR contains a partition table with 4 partition table entries (see description below), each 16 bytes in size, from 0x1BE to 0x1FD (0x1FE and 0x1FF contain the boot signature)
; The MBR code typically does do the following :
; - Relocate to another location (usually 0x0000:0x0600)
; - Examine the 7th bit of the first byte of each partition table entry : If this byte is set then the partition is marked as "active"
; - If exactly one partition has the active bit set, take this partition as the active partition
; - If no partition is active : hang or ask the user to select an active partition, and then optionally mark it as active in the MBR (i.e. save it as active in the actual MBR on disk)
; - If multiple partitions are marked as active : hang or take the first one as "the" active partition or ask the user to select one
; - Load the volume boot record (VBR) i.e. the first sector of the active partition at memort address 0x0000:0x7C00
; - Save the BIOS boot drive ID in DL
; - Save the pointer to the active partition in the relocated MBR in DS:SI
; - Make a far jump to 0x0000:0x7C00 

; The format of a partition table entry is as follows :
;------------------------------------------------------------------------------
;| Offset1 | Size (bytes) | Description                                       |
;|-----------------------------------------------------------------------------
;| 0x00    | 1            | Drive attributes (bit 7 set = active or bootable) |
;| 0x01    | 3            | CHS Address of partition start                    |
;| 0x04    | 1            | Partition type                                    |
;| 0x05    | 3            | CHS address of last partition sector              |
;| 0x08    | 4            | LBA of partition start                            |
;| 0x0C    | 4            | Number of sectors in partition                    |
;------------------------------------------------------------------------------

; First let us include some definitions of constants (the constants themselves are described in comments)

STACK_TOP               equ 0x1000                ; Top of the stack used by the boot loader code
SECTOR_SIZE             equ 0x200                 ; Size of a sector (or size of MBR, VBR)

BOOT_ADDRESS            equ 0x7C00                ; This is where the MBR, VBR is loaded
RELOC_ADDRESS           equ 0x0600                ; This is where the relocated boot sector code/data is placed
PARTITION_TABLE_OFFSET  equ 0x01BE                ; Offset of the start of the partition table in the MBR

SCREEN_TEXT_BUFFER      equ 0xB800                ; Video buffer for the 80x25 VBE text mode

; We need to tell the assembler that all labels need to be resolved relative to the memory address 0x0600 in the binary code

ORG RELOC_ADDRESS

; The x86 system always starts in the REAL mode
; This is the 16-bit mode without any of the protected mode features
; We need to tell the assembler to produce 16-bit code

BITS 16

; This is where the bootloader starts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MBR:

	; We don't want any interrupts right now.
	
	cli

	; We first set up a usable stack at 0x1000

	xor   ax, ax
	mov   ss, ax
	mov   sp, STACK_TOP 

	; BIOS stores system information in certain registers when control is transferred to the boot loader. Save these registers on the stack ; we will need to pass them on
	
	push  dx                 ; DL contains the boot drive ID and DH may contain the INT 0x13 support flag (bit 5)
	push  es                 ; ES:DI points to "$PnP" installation check structure for systems with Plug-and-Play BIOS or BBS support 
	push  di

	; We should initialize the segment registers to the base address values that we want to use (we use 0x0000)
	; First we set DS, ES to 0
	
	mov   ds, ax
	mov   es, ax
	
	; Then, we relocate the MBR code/data to memory location 0x0000:0x0600

	mov   cx, SECTOR_SIZE
	mov   si, BOOT_ADDRESS
	mov   di, RELOC_ADDRESS
	rep   movsb
	jmp   0x0000:Start

	Start:

	; BIOS stores system information in certain registers when control is transferred to the boot loader. Save these registers on the stack ; we will need to pass them on

	mov   [Drive_ID], dl     ; Save the boot drive ID, we will need it when trying to read the VBR from disk

	; Identify the active partition

	GetActivePartition:
	mov   cx, 4
	mov   bx, MBR+PARTITION_TABLE_OFFSET
	mov   ax, [bx]
	test  ax, 0x80
	jnz   LoadVBR
	add   bx, 0x10
	loop  GetActivePartition

	mov   si, Messages.NoActivePart
	jmp   HaltSystem

	LoadVBR:
	mov   [Active_Partition], bx

	mov   ax, [bx+0x08]
	mov   [DAP.Start_Sector], ax
	mov   ax, [bx+0x0A]
	mov   [DAP.Start_Sector+0x02], ax

	; We don't have enough space to write a disk driver to load the VBR, so we will simply use routines provided by the BIOS
	; We will first try the 'extended' INT 0x13 BIOS routines to read from disk using the LBA scheme
	; In the LBA scheme the disk sectors are numbered sequentially as in an array
	; First we need to check that these BIOS extensions exist
	; For that we need to call the INT 0x13, AH=0x41 routine
	; This routine takes the following inputs :
	; - DL : Drive ID 
	; - BX : 0x55AA
	; - AH : 0x41
	; Its results are :
	; - CF : Carry flag set if extension not present, clear if present 
	; - AH : Error code (if carry flag is set) or major version number of the BIOS extension (if carry flag is not set)
	; - BX : 0xAA55 if the routine succeeded
	; - CX : Interface support bitmask
	;        1 -> Device access using the packet structure (this is what's relevant for us)
	;        2 -> Drive locking and ejecting
	;        4 -> Enhanced Disk Drive (EDD) support 
	; If BIOS extensions are supported we will use the INT 0x13, AH=0x42 BIOS routine to read sectors from disk using the LBA scheme 
	; If BIOS extensions are not usable we fall back to the 'old' BIOS routine INT 0x13 AH=0x02 that reads from disk using the CHS scheme	

	mov   bx, 0x55AA
	mov   ah, 0x41
	int   0x13
	jc    DiskReadUsingCHS
	cmp   bx, 0xAA55
	jne   DiskReadUsingCHS
	test  cx, 1
	jz    DiskReadUsingCHS
	
	; We need to provide INT 0x13 AH=0x02 a data structure containing information about what sectors to read, and where to put the read data in memory
	; This data structure is called the Data Address Packet (DAP) and is defined at the end of the boot sector code
	
	DiskReadUsingLBA:
	mov   dl, BYTE [Drive_ID]
	mov   si, DAP
	mov   ah, 0x42
	int   0x13
	jnc   LaunchStage2
	
	; BIOS extensions do not exist or didn't work, and so we need to read from disk using INT 0x13, AH=0x02 that employs the CHS scheme
	; This routine should exist even on older BIOSes
	; However, it has some limitations, most notably the fact that it cannot access very large disks

	DiskReadUsingCHS:	
	mov   bx, [Active_Partition]
	mov   dl, [Drive_ID]
	mov   dh, [bx+1]
	mov   cl, [bx+2]
	mov   ch, [bx+3]
	mov   bx, 0
	mov   es, bx
	mov   bx, BOOT_ADDRESS
	mov   al, 1
	mov   ah, 0x02
	int   0x13
	jnc   LaunchStage2
	mov   si, Messages.DiskReadErr
	jmp   HaltSystem
	
	; We reach here if the disk read was successful 
	
	LaunchStage2:
	mov   sp, STACK_TOP 
	sub   sp, 6
	pop   di
	pop   es
	pop   dx
	mov   dl, [Drive_ID]
	mov   si, 0
	mov   ds, si
	mov   si, [Active_Partition]
	jmp   BOOT_ADDRESS 

	; If the boot strap failed (no active partition or disk read error) then halt the system
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
	mov   cx, 0
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

; Padding of zeros till offset 218 where we should save the disk time stamp

times 218-($-$$)     db 0	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Disk_Time_Stamp: 
times 8              db 0

Active_Partition     dw 0
Drive_ID             db 0

DAP:
.Size                db 0x10
.Unused1             db 0
.Sectors_Count       db 1
.Unused2             db 0
.Memory_Offset       dw BOOT_ADDRESS
.Memory_Segment      dw 0
.Start_Sector        dq 1

Messages:
.NoActivePart        db 'No active partition found', 0
.DiskReadErr         db 'Unable to load VBR from disk', 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Padding of zeroes till we reach (near) the start of the partition tables

times 440-($-$$)     db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Disk_Signature:
.UID                 dd 0xADADADAD
.Protection          dw 0

Partition_Table:

Partition_Table_Entry1:
.Status              db 0x80
.Head_Start          db 0
.Sector_Start        db 0
.Cylinder_Start      db 0
.Type                db 0
.Head_End            db 0
.Sector_End          db 0
.Cylinder_End        db 0
.LBA_Start           dd 1
.LBA_Sectors         dd 0x1000-1

Partition_Table_Entry2:
.Status              db 0
.Head_Start          db 0
.Sector_Start        db 0
.Cylinder_Start      db 0
.Type                db 0
.Head_End            db 0
.Sector_End          db 0
.Cylinder_End        db 0
.LBA_Start           dd 0
.LBA_Sectors         dd 0

Partition_Table_Entry3:
.Status              db 0
.Head_Start          db 0
.Sector_Start        db 0
.Cylinder_Start      db 0
.Type                db 0
.Head_End            db 0
.Sector_End          db 0
.Cylinder_End        db 0  
.LBA_Start           dd 0
.LBA_Sectors         dd 0

Partition_Table_Entry4:
.Status              db 0
.Head_Start          db 0
.Sector_Start        db 0
.Cylinder_Start      db 0
.Type                db 0
.Head_End            db 0
.Sector_End          db 0
.Cylinder_End        db 0  
.LBA_Start           dd 0
.LBA_Sectors         dd 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Padding of zeroes till the end of the boot sector (barring the last two bytes that are reserved for the boot signature)

times 512-2-($-$$)   db 0 

; The last two bytes of the boot sector need to have the following boot signature for BIOS to consider it to be valid

Boot_Signature:
dw   0xAA55

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

