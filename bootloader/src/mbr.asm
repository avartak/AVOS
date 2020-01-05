; What follows is a traditional master boot record (MBR) that is part of the BIOS-based boot sequence of an x86 PC
; - This will not run through the Unified Extensible Firmware Interface (UEFI)
; - Traditional partitions ; no GUID Partition Table (GPT) or support for it 
; - Certain parts of the MBR (the partition table in particular) are expected to be written/maintained by a partition editor

; This file contains the code residing in the master boot record (MBR) of a fixed disk or removable drive that is deemed bootable [boot drive]
; MBR is the first sector (exactly 512 B) of the boot drive
; The last word of this sector has to be 0xAA55 -- this is the boot signature
; The BIOS will look for this signature, then load this sector at memory address 0x7C00, and jump to it
; The boot drive ID is stored in DL --> typically (but not always!) 0x80, 0x81, ... for fixed disks or removable drives ; 0x00, 0x01, ... for floppies ; 0x7E, 0x7F are reserved

; The MBR contains a partition table with 4 partition table entries (see description below), each 16 bytes in size, from 0x1BE to 0x1FD (0x1FE and 0x1FF contain the boot signature)
; The MBR code typically does the following :
; - Relocate to another location (usually 0x0000:0x0600)
; - Examine the 7th bit of the first byte of each partition table entry : If this byte is set then the partition is marked as "active"
; - If exactly one partition has the active bit set, take this partition as the active partition
; - If no partition is active : hang or ask the user to select an active partition, and then optionally mark it as active in the MBR (i.e. save it as active in the actual MBR on disk)
; - If multiple partitions are marked as active : hang or take the first one as "the" active partition or ask the user to select one
; - Load the volume boot record (VBR) i.e. the first sector of the active partition at memory address 0x0000:0x7C00 [VBR is a more OS-specific first stage boot loader]
;   * No check is made of the sector size when loading the boot sector of a volume/partition
;   * Sector size of 512 bytes is assumed but even if it is larger (e.g. 4 KB) we will still only be using the first 512 bytes of the sector content that gets loaded in memory
; - Preserve the BIOS boot drive ID in DL
; - Preserve the contents of ES:DI ("$PnP" installation check structure) --> These may be needed by the OS downstream
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

; First let us include some definitions of constants

%include "bootloader/include/bootinfo.inc"          ; Common boot related information 

MBR_SIZE                equ 0x0200                  ; Size of the MBR
MBR_RELOC_ADDRESS       equ 0x0600                  ; This is where the MBR relocates itself to before loading the VBR 
STACK_TOP               equ 0x7C00                  ; Top of the stack used by the MBR
PARTITION_TABLE_OFFSET  equ 0x01BE                  ; Offset of the start of the partition table in the MBR (or some VBRs) : byte 446

; We need to tell the assembler that all labels need to be resolved relative to MBR_RELOC_ADDRESS in the binary code

ORG MBR_RELOC_ADDRESS

; The x86 system always starts in the REAL mode
; This is the 16-bit mode without any of the protected mode features
; We need to tell the assembler to produce 16-bit code

BITS 16

; This is where the bootloader starts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MBR:

	; We don't want any interrupts right now.
	
	cli

	; First set up a usable stack 

	xor   ax, ax
	mov   ss, ax
	mov   sp, STACK_TOP 

	; BIOS stores system information in certain registers when control is transferred to the boot loader. Save these registers on the stack ; we will need to pass them on
	
	push  dx                                        ; DL contains the boot drive ID and DH may contain the INT 0x13 support flag (bit 5)
	push  es                                        ; ES:DI points to "$PnP" installation check structure for systems with Plug-and-Play BIOS or BBS support 
	push  di

	; We should initialize the segment registers to the base address values that we want to use (we use 0x0000)
	; First we set DS, ES to 0
	
	mov   ds, ax
	mov   es, ax

	; Then, we relocate the MBR code/data to memory location 0x0000:MBR_RELOC_ADDRESS

	mov   cx, MBR_SIZE
	mov   si, VBR_ADDRESS
	mov   di, MBR_RELOC_ADDRESS
	cld                                             ; Clear the direction flag so that MOVSB proceeds from low to high memory addresses
	rep   movsb
	jmp   0x0000:Start

	Start:

	; We can reenable interrupts now

	sti
	
	; Identify the active partition

	mov   cx, 4
	mov   bx, MBR+PARTITION_TABLE_OFFSET
	GetActivePartition:
	mov   al, [bx]
	cmp   al, 0x80
	je    LoadVBR
	add   bx, 0x10
	loop  GetActivePartition

	; No active partition was found
	; This drive does not seem to be bootable
	; Halt

	mov   si, Messages.NoActivePart
	jmp   HaltSystem

	LoadVBR:
	push  bx                                        ; Save the start address of the active partition in the relocated MBR. This will eventually be passed on through DS:SI

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

	mov   ax, [bx+0x08]                             ; Save the LBA address of the start sector of the active partition in the Disk Address Packet used by extended INT 0x13 
	mov   [DAP.Start_Sector], ax
	mov   ax, [bx+0x0A]                             ; We could have just copied the 4 bytes of LBA into EAX and moved them to the DAP
	mov   [DAP.Start_Sector+0x02], ax

	mov   bx, 0x55AA
	mov   ah, 0x41
	int   0x13
	jc    DiskReadUsingCHS
	cmp   bx, 0xAA55
	jne   DiskReadUsingCHS
	test  cx, 1
	jz    DiskReadUsingCHS
	
	; We need to provide INT 0x13 AH=0x42 a data structure containing information about what sectors to read, and where to put the read data in memory
	; This data structure is called the Data Address Packet (DAP) and is defined at the end of the boot sector code
	
	DiskReadUsingLBA:
	mov   si, DAP
	mov   dl, [STACK_TOP-2]
	mov   ah, 0x42
	int   0x13
	jnc   CheckVBR
	
	; BIOS extensions do not exist or didn't work, and so we need to read from disk using INT 0x13, AH=0x02 that employs the CHS scheme
	; This routine should exist even on older BIOSes
	; However, it has some limitations, most notably the fact that it cannot access very large disks
	; It takes as input
	; - Drive ID in DL
	; - Start sector number ranging between 1-63 (6-bits) in CL (bits 0 to 5)
	; - Start cylinder number ranging between 0-1023 (10-bits) in CH and CL (bits 8,9 of cylinder number go in bits 6,7 of CL)
	; - Start head number in DH
	; - Number of sectors to read in AL
	; - Memory address to load into is stored in ES:BX
	; - AH contains return code of the read routine ; AL contains the actual number of sectors that got read ; Carry flag is clear if the read was successful  

	DiskReadUsingCHS:	
	mov   dl, [STACK_TOP-2]
	mov   bx, [STACK_TOP-8]
	mov   dh, [bx+1]
	mov   cl, [bx+2]
	mov   ch, [bx+3]
	mov   bx, VBR_ADDRESS
	mov   al, 1
	mov   ah, 0x02
	int   0x13
	jnc   CheckVBR
	mov   si, Messages.DiskReadErr
	jmp   HaltSystem
	
	; We reach here if the disk read was successful 
	; Now check if the loaded VBR has the boot signature at the end
	
	CheckVBR:
	mov   ax, [VBR_ADDRESS+510]
	cmp   ax, 0xAA55
	je    LaunchVBR
	mov   si, Messages.InvalidVBR
	jmp   HaltSystem

	; We are all set to launch into the VBR

	LaunchVBR:
	mov   sp, STACK_TOP-8
	pop   si
	pop   di
	pop   es
	pop   dx
	jmp   VBR_ADDRESS 

	; If the boot strap failed (no active partition or disk read error) then halt the system
	; Before halting we print an error message on the screen 
	; We use INT 0x10 AH=0x0E to print a string to screen character-by-character
	; The character byte is stored in AL ; BH contains the display page number ; BL contains the display color for the character

	HaltSystem:
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

; Padding of zeros till offset 218 where we should save the disk time stamp

times 218-($-$$)     db 0	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

times 2              db 0                           ; Saving 0x0000 word (seems to be suggested for a standard MBR)
Original_Drive_ID    db 0x80                        ; Save the ID of the original drive on which the MBR is written [To be edited by MBR software]
Disk_Time_Stamp:                                    ; 3-bytes for the disk timestamp (1st byte seconds ; 2nd byte minutes ; 3rd byte hours) [To be edited by MBR software]
times 3              db 0                         

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DAP:
.Size                db 0x10                        ; Size of the DAP - this should always be 0x10
.Unused1             db 0                           ; Reserved
.Sectors_Count       db 1                           ; Number of sectors to read (we need to read just the one sector containing the VBR)
.Unused2             db 0                           ; Reserved
.Memory_Offset       dw VBR_ADDRESS                 ; Offset of the memory location where the data from disk will be copied to 
.Memory_Segment      dw 0                           ; Segment address corresponding to the memory location where the data from disk will be copied to 
.Start_Sector        dq 0                           ; Starting sector (in LBA) on disk for read

Messages:
.NoActivePart        db 'No bootable partition found', 0
.DiskReadErr         db 'Unable to load volume boot record', 0
.InvalidVBR          db 'Invalid volume boot record', 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Padding of zeroes till we reach (near) the start of the partition tables [there are 6 bytes before the partition table containing some ID information]

times PARTITION_TABLE_OFFSET-6-($-$$)     db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Disk_Signature:
.UID                 dd 0                           ; unique disk ID [To be edited by MBR software]
.Protection          dw 0                           ; 0x5A5A if copy protected, else 0x0000 [To be edited by MBR software]

Partition_Table:                                    ; [Everything below to be edited by MBR software]

Partition_Table_Entry1:                             ; [All entries to be edited by MBR software]
.Status              db 0x80                        ; Active partition has this byte set to 0x80, other partitions have this byte set to 0
.Head_Start          db 0                           ; 3 bytes corresponding to the CHS of the starting sector of the partition
.Sector_Start        db 0
.Cylinder_Start      db 0
.Type                db 0                           ; Partition type - set to 0 in our case
.Head_End            db 0                           ; 3 bytes corresponding to the CHS of the last sector of the partition (not used by us)
.Sector_End          db 0
.Cylinder_End        db 0
.LBA_Start           dd PARTITION_START_LBA         ; LBA of the starting sector
.LBA_Sectors         dd 0xFFFFFFFF                  ; Number of sectors in the partition

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

times MBR_SIZE-2-($-$$)   db 0 

; The last two bytes of the boot sector need to have the following boot signature for BIOS to consider it to be valid

Boot_Signature       dw 0xAA55

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

