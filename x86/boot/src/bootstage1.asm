; Boot loader code in the master boot record (MBR) of a bootable drive
; This is a single sector (exactly 512 B) of code loaded from the first sector of the boot drive
; The last word of this sector has to be 0xAA55 -- this is the boot signature
; The BIOS will look for this signature and then load the 512 B at the memory location 0x7C00

; First let us include some definitions of constants (the constants themselves are described in comments)

STACK_TOP               equ 0x7000                                      ; Top of the stack - it can extend down till 0x500 without running into the BIOS data area (0x400-0x500)
BOOT2_DISK_START        equ 1                                           ; Starting sector of the 2nd stage of the boot loader on disk
BOOT2_SIZE              equ 0x20 - 1                                    ; Size of the 2nd stage of the boot loader in sectors (512 B)

; We need to tell the assembler that all labels need to be resolved relative to the memory address 0x7C00 in the binary code

section .boot

; The x86 system always starts in the REAL mode
; This is the 16-bit mode without any of the protected mode features
; We need to tell the assembler to produce 16-bit code

BITS 16

; This is where the bootloader starts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global BootStage1
BootStage1:

	; The 512 B available here are too few to do much in terms of loading an OS
	; The code in the boot sector will simply load the 2nd stage from disk to the memory location 0x8000
	; The 2nd stage will then set up the 32-bit system in protected mode and load the kernel
	; Alright, lets get started. We don't want any interrupts right now.
	
	cli
	
	; Physical address given by reg:add combination is : [reg] x 0x10 + add
	; We should initialize the segment registers to the base address values that we want to use
	; We should not assume the segment registers to be initialized in a certain way when we are handed control
	; The segment registers in 16-bit environment are : CS, DS, ES, SS. We will set all of them to 0
	; First the code segment (CS) -- The only way to change the value of the code segment register is by performing a long jump
	
	jmp   0x0000:Start
	Start:
	
	; Then, lets set DS, ES and SS to 0, and the stack pointer (SP) at 0x7000
	
	xor   ax, ax
	mov   ds, ax
	mov   es, ax
	mov   ss, ax
	mov   sp, STACK_TOP 
	
	; The first stage of the boot loader needs to load the second stage from disk to memory and jump to it
	; So we need some code that does the reading from disk
	; We don't have enough space to write a disk driver here, so we will simply use routines provided by the BIOS
	; We will first try the 'extended' BIOS routines to read from disk using the LBA scheme
	; In the LBA scheme the disk sectors are numbered sequentially as in an array
	; First we need to check that these BIOS extensions exist
	; For that we need to call the INT 0x13, AH=0x41 routine
	; This routine takes the following inputs :
	; - DL : Drive ID (Note : BIOS stores the drive ID in DL when control is transferred to the boot loader)
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
	; If BIOS extension is not usable we fall back to the 'old' BIOS routine that reads from disk using the CHS scheme	

	mov   [Drive_ID], dl
	mov   bx, 0x55AA
	mov   ah, 0x41
	int   0x13
	jc    DiskReadUsingCHS
	cmp   bx, 0xAA55
	jne   DiskReadUsingCHS
	test  cx, 1
	jz    DiskReadUsingCHS
	
	; We will use the INT 0x13, AH=0x42 BIOS routine to read sectors from disk using the LBA scheme
	; We need to provide this routine a data structure containing information about what sectors to read, and where to put the read data in memory
	; This data structure is called the Data Address Packet (DAP) and is defined at the end of the boot sector code
	
	DiskReadUsingLBA:
	mov   dl, BYTE [Drive_ID]
	mov   si, Disk_Address_Packet
	mov   ah, 0x42
	int   0x13
	jnc   LaunchStage2
	
	; If BIOS extensions do not exist we will need to read from disk using INT 0x13, AH=0x02 that employs the CHS scheme
	; This routine should exist even on older BIOSes
	; However, it has some limitations, most notably the fact that it cannot access very large disks
	; Some details about conversion from LBA to CHS
	; Temp     = LBA % (Heads * Sectors_Per_Track)
	; Cylinder = LBA / (Heads * Sectors_Per_Track)
	; Head     = Temp / Sectors_Per_Track
	; Sector   = Temp % Sectors_Per_Track + 1	
	
	; First we read the disk geometry (number of cylinders, heads and sectors per track) using the BIOS routine INT 0x13, AH=0x08
	; This routing requires the drive ID to be stored in DL
	; To work around some buggy BIOSes, make sure that ES:DI is 0:0
	; The results of the routine are :
	; - CF : Carry flag set if extension not present, clear if present
	; - AH : Return code
	; - DL : Number of hard disk drives
	; - DH : Logical last index of heads (or number of heads - 1)
	; - CX : First 6 bits store the number of sectors per track, highest 10 bits store the last logical index of the cylinders (or number of cylinders - 1)
	; - BL : Drive type (only AT/PS2 floppies) 
	; - ES:DI --> Pointer to drive parameter table (only for floppies) 
	
	DiskReadUsingCHS:
	xor   ax, ax
	mov   es, ax
	mov   di, ax
	mov   dl, BYTE [Drive_ID]
	mov   ah, 0x08
	int   0x13
	jc    HaltSystem
	
	add   dh, 0x1
	mov   [Heads], dh
	and   cl, 0x3F
	mov   [Sectors_Per_Track], cl
	mov   al, cl
	mul   dh
	mov   [Sectors_Per_Cylinder], ax
	
	; This is the code that converts the address of the starting sector that we want to read from the LBA scheme to the CHS scheme 
	; Look at the translation formulas above to understand what's being done
	; Note that : Sectors_Per_Cylinder = Heads * Sectors_Per_Track
	
	mov   eax, BOOT2_DISK_START
	mov   edx, 0
	movzx ebx, WORD [Sectors_Per_Cylinder]
	div   ebx 
	cmp   eax, 0x3FF
	jg    HaltSystem
	shl   ax, 0x6
	mov   cx, ax
	
	mov   ax, dx
	div   BYTE [Sectors_Per_Track]
	add   ah, 0x1
	and   ah, 0x3F
	or    cl, ah
	
	mov   dh, al

	; Store the memory location where to copy from disk in ES:BX
	; Number of sectors to copy are stored in AL
	; Drive ID is stored in DL as always

	xor   ax, ax
	mov   es, ax
	mov   bx, BOOT2_START
	mov   dl, BYTE [Drive_ID]
	mov   al, BOOT2_SIZE

	; Now call INT 0x13, AH=0x02
	
	mov   ah, 0x02
	int   0x13
	jc    HaltSystem
	
	; We reach here if the disk read was successful 
	
	LaunchStage2:
	mov   dl, [Drive_ID]
	jmp   BOOT2_START 
	


	; If we did not read what we wanted to we halt
	; Before halting we print an error message on the screen 
	; In the PC architecture the video buffer sits at 0xB8000
	; We put the ES segment at that location
	; We can write to the video buffer as if it is array of characters (in fact it's an array of the character byte + attribute byte)
	; the usual VGA compatible text mode has 80 columns (i.e. 80 characters per line) and 25 rows (i.e. 25 lines)
	; We will print the error message on the penultimate line, in red
	
	HaltSystem:
	mov   ax, 0xB800         
	mov   es, ax             
	mov   di, 80*23*2        
	mov   si, Halt_Message   
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
	jmp   HaltSystem

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Disk_CHS_Geometry:
Drive_ID             db 0
Heads                db 2
Sectors_Per_Track    db 18
Sectors_Per_Cylinder dw 18

Disk_Address_Packet:
DAP_Size             db 0x10
DAP_Unused1          db 0
DAP_Sectors_Count    db BOOT2_SIZE
DAP_Unused2          db 0
DAP_Memory_Offset    dw BOOT2_START
DAP_Memory_Segment   dw 0
DAP_Start_Sector     dq BOOT2_DISK_START

Messages:
Halt_Message         db 'Unable to read AVOS from disk', 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Padding of zeroes till the end of the boot sector (barring the last two bytes that are reserved for the boot signature)

times 512-2-($-$$) db 0 

; The last two bytes of the boot sector need to have the following boot signature for BIOS to consider it to be valid

Boot_Signature:
dw   0xAA55

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Some notes about the mapping of the first MB of the physical memory in the PC architecture, and how we use this memory 

; 0x000000 - 0x000400 : Interrupt Vector Table  
; 0x000400 - 0x000500 : BIOS data area          
; 0x000500 - 0x007C00 : Free area         
; 0x007C00 - 0x007E00 : Boot sector             
; 0x007E00 - 0x09FC00 : Free area               
; 0x09FC00 - 0x0A0000 : Extended bios data area 
; 0x0A0000 - 0x0C0000 : Video memory            
; 0x0C0000 - 0x100000 : BIOS            

; We set the top of the stack at 0x7000; it can go down to 0x1000 allowing for 24 KB (plenty)
; 0x001000 - 0x007000

; The first stage of our boot loader will (rather has to) reside at :
; 0x007C00 - 0x007E00

; We will put the second stage of our boot loader at 0x8000 and allocate 32 KB of memory for it 
; 0x008000 - 0x010000


BOOT2_START:
