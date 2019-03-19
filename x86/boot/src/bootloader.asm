; Boot loader code in the master boot record (MBR) of a bootable drive
; This is a single sector (exactly 512 B) of code loaded from the first sector of the boot drive
; The last word of this sector has to be 0xAA55 -- this is the boot signature
; The BIOS will look for this signature and then load the 512 B at the memory location 0x7C00
; This is specified by the ORG command below 

; First let us include some definitions of constants (the constants themselves are described in comments)

%include "x86/boot/src/defs.asm"

; We need to tell the assembler that all labels need to be resolved relative to the memory address 0x7C00 in the binary code

ORG START_BOOT1

; The x86 system always starts in the REAL mode
; This is the 16-bit mode without any of the protected mode features
; We need to tell the assembler to produce 16-bit code

BITS 16

; This is where the bootloader starts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Boot:

	; The 512 B available here are too few to do much in terms of loading an OS
	; The code in the boot sector will simply load the 2nd stage from the floppy drive (specifically, logical address of 4 KB - 20 KB, so 16 KB, on the floppy drive) to the memory location 0x8000
	; This is meant to be the 2nd stage of the boot process. It will set up the 32-bit system (protected mode) and load the kernel
	; Alright, lets get started. We don't want any interrupts right now. First lets set up our segments and stack, and then we reenable the interrupts

	cli

	; Physical address given by reg:add combination is : [reg] x 0x10 + add
	; We should initialize the segment registers to the base address values that we want to use
	; We should not assume the segment registers to be initialized in a certain way when we are handed control
	; The segment registers in 16-bit environment are : CS, DS, ES, SS. We will set all them to 0
	; First code segment (CS) -- The only way to change the value of the code segment register is by performing a long jump

	jmp 0x0000:Start
	Start:

	; Then, lets set DS, ES and SS to 0, and the stack pointer (SP) at 0x7000

	xor  ax, ax
	mov  ds, ax
	mov  es, ax
	mov  ss, ax
	mov  sp, STACK_TOP 

	; The first stage of the boot loader needs to load the second stage from disk to memory and jump to it
	; So we need some code that does the reading from disk
	; We don't have enough space to write a disk driver here, so we will simply use routine(s) provided by the BIOS
	; We will use the 'extended' BIOS routines to read from disk using the LBA scheme
	; First we need to check that these BIOS extensions exist (they won't on old machines but we are not planning to run on them or be compatible with old hardware in general)
	; For that we need to call the INT 0x13, AH=0x41 routine

	mov  dl, HDD_ID                                              ; BIOS is expected to put the ID of the boot drive in DL, but not sure how reliable that is. We set the drive ID in DL by hand
    mov  bx, 0x55AA
    mov  ah, 0x41
    int  0x13
    jc   HaltSystem
    cmp  bx, 0xAA55
    jne  HaltSystem
    test cx, 1
    jz   HaltSystem

	; We will use the INT 0x13, AH=0x42 BIOS routine to read sectors from disk using the LBA scheme
	; We need to provide this routine a data structure containing information about what sectors to read, and where to put the read data in memory
	; This data structure is called the Data Address Packet (DAP) and is defined at the end of the boot sector code

    mov  dl, HDD_ID
    mov  si, Disk_Address_Packet
    mov  ah, 0x42
    int  0x13
    jc   HaltSystem

	LaunchStage2:
	jmp  START_BOOT2 

	; If we did not read what we wanted to we halt
	HaltSystem:
	cli
	hlt
	jmp HaltSystem


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Disk_Address_Packet:
DAP_Size             db 0x10
DAP_Unused1          db 0
DAP_Sectors_Count    db SIZE_BOOT2/SECTOR_SIZE
DAP_Unused2          db 0
DAP_Memory_Offset    dw START_BOOT2
DAP_Memory_Segment   dw 0
DAP_Start_Sector     dq START_BOOT2_DISK

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Adding a zero padding to the boot sector

times SIZE_BOOT1-2-($-$$) db 0 

; The last two bytes of the boot sector need to have the following boot signature 
; Otherwise BIOS will not recognize this as a boot sector

dw BOOTSECTOR_MAGIC



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

; We set the top of the stack at 0x7000; it can go down to 0x500 allowing for 26 KB (plenty)
; 0x000500 - 0x007000

; We allocate 1 KB buffer from 0x7000 to 0x7400; it can extend till 0x7C00 (3 KB) if needed
; 0x007000 - 0x007C00

; The first stage of our boot loader will (rather has to) reside at :
; 0x007C00 - 0x007E00

; We use the next 512 bytes to set up a GDT
; 0x007E00 - 0x008000

; We will put the second stage of our boot loader at 0x8000 and allocate 32 KB of memory for it 
; 0x008000 - 0x010000

; And then any other tables will start from 0x10000

