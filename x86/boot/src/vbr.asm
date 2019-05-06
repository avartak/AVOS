; Boot loader code in the volume boot record (VBR) of a bootable drive
; This is a single sector (512 B) located at the start of an active partition of a bootable drive
; It is loaded at 0x0000:0x7C00 by the master boot record (MBR)
; It also has the boot signature word at its end, just like the MBR
; The VBR does not contain a partition table, and so has a few more bytes of code at its disposal, but this is still not much
; It loads the boot loader into memory and transfers control to it. The boot loader then loads the OS

; First let us include some definitions of constants (the constants themselves are described in comments)

STACK_TOP               equ 0x7000                                      ; Top of the stack - it can extend down till 0x500 without running into the BIOS data area (0x400-0x500)

BOOTLOADER_DISK_START   equ 2                                           ; Starting sector of the boot loader on disk
BOOTLOADER_SIZE         equ 0x40 - 1                                    ; Size of the boot loader in sectors (512 B)

SCREEN_TEXT_BUFFER      equ 0xB800                                      ; Video buffer for the 80x25 VBE text mode

; We need to tell the assembler that all labels need to be resolved relative to the memory address 0x7C00 in the binary code

ORG 0x7C00

; The x86 system is still in real mode (typically) when control is transferred to the VBR
; We need to tell the assembler to produce 16-bit code

BITS 16

; This is where the VBR boot code starts
; See comments in the MBR code to understand the code used to load the boot loader from disk

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VBR:

	; We don't want any interrupts right now.
	
	cli
	
	; We first set up a usable stack at 0x7000

	xor   ax, ax
	mov   ss, ax
	mov   sp, STACK_TOP 

	; BIOS stores the boot drive ID in DL and the active partition table entry in the relocated MBR in DS:SI when control is transferred to the boot loader. 
	; Save these registers on the stack

	push  dx
	push  ds
	push  si
	
	; The MBR is expected to make a jump to 0x0000:0x7C00 (as opposed to 0x07C0:0x0000 or something else) but we should not assume this
	; We should initialize the segment registers to the base address values that we want to use (0x0000)
	
	jmp   0x0000:Start
	Start:
	
	mov   ds, ax
	mov   es, ax
	
	; MBR retains the drive ID in DL when control is transferred to the VBR. Store this value

	mov   [Drive_ID], dl

	; Check for BIOS extensions to read from disk using the LBA scheme

	mov   bx, 0x55AA
	mov   ah, 0x41
	int   0x13
	jc    DiskReadUsingCHS
	cmp   bx, 0xAA55
	jne   DiskReadUsingCHS
	test  cx, 1
	jz    DiskReadUsingCHS
	
	; BIOS extensions exists. So, we use the INT 0x13, AH=0x42 BIOS routine to read from disk
	
	DiskReadUsingLBA:
	mov   dl, BYTE [Drive_ID]
	mov   si, DAP
	mov   ah, 0x42
	int   0x13
	jnc   LaunchStage2
	
	; If BIOS extensions do not exist or did not work, we will need to read from disk using INT 0x13, AH=0x02 that employs the CHS scheme
	; First we read the disk geometry
	
	DiskReadUsingCHS:
	xor   ax, ax
	mov   es, ax
	mov   di, ax
	mov   dl, BYTE [Drive_ID]
	mov   ah, 0x08
	int   0x13
	jc    HaltSystem
	
	add   dh, 0x1
	mov   [CHS_Geometry.Heads], dh
	and   cl, 0x3F
	mov   [CHS_Geometry.Sectors_Per_Track], cl
	mov   al, cl
	mul   dh
	mov   [CHS_Geometry.Sectors_Per_Cylinder], ax
	
	; This is the code that converts the address of the starting sector from the LBA scheme to the CHS scheme 
	; It stores the C,H,S values is appropriate registers and then calls INT 0x13, AH=0x42

	mov   eax, BOOTLOADER_DISK_START
	mov   edx, 0
	movzx ebx, WORD [CHS_Geometry.Sectors_Per_Cylinder]
	div   ebx 
	cmp   eax, 0x3FF
	jg    HaltSystem
	shl   ax, 0x6
	mov   cx, ax
	
	mov   ax, dx
	div   BYTE [CHS_Geometry.Sectors_Per_Track]
	add   ah, 0x1
	and   ah, 0x3F
	or    cl, ah
	
	mov   dh, al

	; Store the load memory address in ES:BX
	; Number of sectors to copy are stored in AL
	; Drive ID is stored in DL as always

	xor   ax, ax
	mov   es, ax
	mov   bx, BOOTLOADER_START
	mov   dl, BYTE [Drive_ID]
	mov   al, BOOTLOADER_SIZE

	; Now call INT 0x13, AH=0x02
	
	mov   ah, 0x02
	int   0x13
	jc    HaltSystem
	
	; We reach here if the disk read was successful 
	
	LaunchStage2:
	mov   sp, STACK_TOP
	sub   sp, 0x6
	pop   si
	pop   bx
	pop   dx
	jmp   BOOTLOADER_START 
	
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

Drive_ID              db 0
CHS_Geometry:
.Heads                db 2
.Sectors_Per_Track    db 18
.Sectors_Per_Cylinder dw 18

DAP:
.Size                 db 0x10
.Unused1              db 0
.Sectors_Count        db BOOTLOADER_SIZE
.Unused2              db 0
.Memory_Offset        dw BOOTLOADER_START
.Memory_Segment       dw 0
.Start_Sector         dq BOOTLOADER_DISK_START

Messages:
.DiskIOErr            db 'Unable to read AVOS from disk', 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Padding of zeroes till offset 446 (location of the partition table in the MBR). We will keep this space empty

times 446-($-$$) db 0

Volume_Partition_Table:
times 64 db 0

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
; 0x000600 - 0x000800 : Relocated MBR
; 0x000800 - 0x001000 : Stack of the relocated MBR
; 0x001000 - 0x007000 : Stack of the boot loader
; 0x007C00 - 0x007E00 : VBR             
; 0x007E00 - 0x09FC00 : Free area
; 0x007E00 - 0x008000 : 16-bit part of the boot loader code
; 0x008000 - 0x00FFFF : 32-bit part of the boot loader code     
; 0x09FC00 - 0x0A0000 : Extended bios data area 
; 0x0A0000 - 0x0C0000 : Video memory            
; 0x0C0000 - 0x100000 : BIOS            

BOOTLOADER_START:
