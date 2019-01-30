; Master boot record (MBR) of the boot loader for AVOS
; This is a single sector (exactly 512 B) of code loaded from the first sector of the boot drive
; The last word of this sector has to be 0xAA55 -- this is the boot signature
; The BIOS will look for this signature and then load the 512 B at the memory location 0x7C00
; We need to tell the assembler (NASM) that all labels need to be based relative to 0x7C00 when it produces a binary for this sector
; This is specified by the ORG command below 

; First let us include some definitions of constants (the constants themselves are described in comments)

%include "x86/boot/src/defs.asm"

; Tell NASM that the code will be loaded at 0x7C00

ORG START_BOOT1

; The x86 system always starts in the REAL mode
; This is the 16-bit mode without any of the protected mode features
; We need to tell the assembler to produce 16-bit code

BITS 16

; This is where the bootloader starts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Boot:

	; We should initialize the segment registers to the base-address values that we want to use
	; We should not assume the segment registers to be initialized in a certain way when we are handed control
	; The segment registers known in 16-bit environment are : CS, DS, ES, SS
	; Physical address given by reg:add combination is : [reg] x 0x10 + add
	; This allows us to address a memory range starting from 0 to 0xFFFF0+0xFFFF
	; If you do the math you will find that this range exceeds the 1 MB address space that a 20-bit address bus can physically access
	; By default the addresses beyond the 1 MB mark get looped back to 0
	; However, in reality we now have processors capable of physically accessing addresses beyond 1 MB
	; To enable access to this >1MB address space, we would need to enable the A20 line
	; This will be done later

	; Now, this is how the 'bootstrap' will happen
	; The 512 B available here are too few to do much in terms of loading an OS
	; The code in the boot sector will simply load the 2nd stage from the floppy drive (specifically, logical address of 4 KB - 20 KB, so 16 KB, on the floppy drive) to the memory location 0x8000
	; This is meant to be the 2nd stage of the boot process. It will set up the 32-bit system (protected mode) and load the kernel
	; Lets take note of the mapping of the first MB of the physical memory in the PC architecture. 

	; 0x000000 - 0x000400 : Interrupt Vector Table  
	; 0x000400 - 0x000500 : BIOS data area          
	; 0x000500 - 0x007C00 : Free area         
	; 0x007C00 - 0x007E00 : Boot sector             
	; 0x007E00 - 0x09FC00 : Free area               
	; 0x09FC00 - 0x0A0000 : Extended bios data area 
	; 0x0A0000 - 0x0C0000 : Video memory            
	; 0x0C0000 - 0x100000 : BIOS            

	; We set the top of the stack at 0x7000; it can go down to 0x500 allowing for 26 KB (plenty)
	; 0x000500 - 0x007000

	; We a assume 1 KB buffer from 0x7000 to 0x7400; it can extend till 0x7C00 (3 KB) if needed
	; 0x007000 - 0x007C00

	; The first stage of our boot loader will (rather has to) reside at :
	; 0x007C00 - 0x007E00

	; We use the next 512 bytes to set up a GDT (and if we really want to, a TSS entry although there is no reason for having it in the boot stage)
	; 0x007E00 - 0x008000

	; We will put the second stage of our boot loader next at 0x8000 and allow it a space of 32 KB :
	; 0x008000 - 0x010000

	; And then any other tables will start from 0x10000

	; Note that there is free space from 0x500 to 0x7C00 as well in case it is needed 
	; We use 16 KB of scratch space as follows
	; 0x001000 - 0x005000 

	; Alright, lets get started. We don't want any interrupts right now. First lets set up our segments and stack, and then we reenable the interrupts

	cli

	; We set all the segment registers to 0
	; First CS

	jmp 0x0000:Start

Start:

	; Then, lets set DS, ES and SS to 0, and the stack pointer (SP) at 0x7000

	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, TOP_STACK 

	; The first stage of the boot loader needs to load the second stage from disk to memory and jump to it
	; So we need to set up some code that does the reading from disk

	; Next we save some information (called the disk address packet) that INT 0x13, AH=0x41 will need to read from disk
	; INT 0x13, AH=0x41 is our preferred function to read from disk
	; If we can use it, we will not have to bother with the disk geometry (cylinders, heads, sectors and what not) but simply use the logical block address (LBA) which is a simple, linear sequence of sectors

	jmp ReadStage2

    Disk_Address_Packet:
    DAP_Size             db 0x10
    DAP_Unused           db 0
    DAP_Sectors_Count    dw SIZE_BOOT2/SECTOR_SIZE
    DAP_Start_Offset     dw START_BOOT2
    DAP_Start_Segment    dw 0
    DAP_Start_Sector     dq 1


ReadStage2:

	; We can reenable the interrupts now
	
	sti

	; We move the drive ID of the boot drive to the register DL from where INT 0x13 reads it, and also store it in a persistent location
	; BIOS is expected to put the ID of the boot drive in DL, but not sure how reliable that is
	; We assume to know where this code will be booting from
	
	mov [Drive], BYTE HDD_ID

	; First we need to test if BIOS supports the AH=0x41 extension; not all of them do
	; For that we need to call INT 0x13, AH=0x41
	; It's input parameters are as follows
	; AH = 0x41 (Duh!)
	; DL = Drive index
	; BX = 0x55AA
	; The output parameters are as follows
	; CF = Set on not present, clear if present
	; AH = Error code or major version
	; BX = 0xAA55
	; CX = Bit 1 : Device Access using the packet structure ; Bit 2 : Drive Locking and Ejecting ; Bit 3 : Enhanced Disk Drive Support (EDD) ; We are only concerned with the first bit being set 

	mov ah, 0x41
	mov bx, 0x55AA
	int 0x13
	jc  .readusingchs
	cmp bx, 0xAA55
	jne .readusingchs
	test cx, 1
	jz  .readusingchs

	.readusinglba:
	mov ah, 0x42
	mov dl, [Drive]
	mov si, Disk_Address_Packet

	int 0x13
	jc .readusingchs
	jmp LaunchStage2


	; Sigh, we don't have the BIOS extension need to read from the disk using the LBA
	; So have to read the disk using INT 0x13, AH=0x02
	; This BIOS service needs the disk geometry
	; So we will first call a service -- INT 0x13, AH=0x08 to read the disk geometry and save it (ReadDriveParameters)
	; We will them call the ReadDriveParameters function to read from the disk
	; These functions are defined in biosio.asm
 
	.readusingchs:
	call ReadDriveParameters

	push START_BOOT2                          ; Where to put the 2nd stage of the boot loader in memory ?
	push SIZE_BOOT2/SECTOR_SIZE               ; How many sectors of the disk do we want to read ?
	push START_BOOT2_DISK                     ; Start sector from where we start the read on the disk
	call ReadSectorsFromDrive

	mov al, [Sectors_Read_Last]               ; Did we really read everything ? 
	cmp al, SIZE_BOOT2_DISK
	je  LaunchStage2
	jmp HaltSystem




LaunchStage2:
	jmp 0x0:START_BOOT2 

HaltSystem:
	cli
	hlt                                       ; If we did not read what we wanted to we halt


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "x86/boot/src/biosio.asm"            ; ReadDriveParameters and ReadSectorsFromDrive are defined in this file

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Adding a zero padding to the boot sector

times SIZE_BOOT1-2-($-$$) db 0 

; The last two bytes of the boot sector need to have the following boot signature 
; Otherwise BIOS will not recognize this as a boot sector

dw BOOTSECT_MAGIC

