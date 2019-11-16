; The Volume Boot Record (VBR) is the first sector of the 'active' or bootable partition that contains our OS
; The Master Boot Record (MBR) first identifies the active partition, then loads the VBR at memory address 0x7C00, and transfers control to it
; The VBR also has the boot signature word (0xAA55) at its end, just like the MBR
; The VBR is the first OS-specific code. It reads our bootloader from disk into memory and transfers control to it 
; Since the VBR is just one sector (512 bytes) long it cannot access filesystems. Therefore, it reads the bootloader from disk knowing the low-level LBA of the bootloader

; First let us include some definitions of constants that the VBR needs

STACK_TOP               equ 0x7C00                      ; Top of the stack
SCREEN_TEXT_BUFFER      equ 0xB800                      ; Video buffer for the 80x25 VBE text mode (for displaying error messages)
BOOTLOADER_PART_START   equ 4                           ; Starting sector of the boot loader on disk
BOOTLOADER_SIZE         equ 0x40 - 1                    ; Size of the boot loader in sectors (512 B)

; We need to tell the assembler that all labels need to be resolved relative to the memory address 0x7C00 in the binary code

ORG 0x7C00

; The x86 system is still in real mode when control is transferred to the VBR
; We need to tell the assembler to produce 16-bit code

BITS 16

; This is where the VBR boot code starts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VBR:

	; We don't want any interrupts right now.
	
	cli
	
	; Clear the direction flag so that the string operations (that we will use later) proceed from low to high memory addresses

	cld

	; We first set up a usable stack at 0x7000

	xor   ax, ax
	mov   ss, ax
	mov   sp, STACK_TOP 

	; BIOS stores the boot drive ID in DL and the active partition table entry in the relocated MBR in DS:SI when control is transferred to the boot loader. 
	; Save these registers on the stack

	push  dx
	push  ds
	push  si
	
	mov   ebx, DWORD [si+0x08]                          ; Get the LBA of the start sector of this partition
	add   ebx, BOOTLOADER_PART_START                    ; Get the LBA of the start sector of the bootloader by adding the offset of the bootloader w.r.t. the first partition sector

	; The MBR is expected to make a jump to 0x0000:0x7C00 (as opposed to 0x07C0:0x0000 or something else) but lets not assume this
	; We should initialize the segment registers to the base address values that we want to use (0x0000)
	
	mov   ds, ax
	mov   es, ax
	jmp   0x0000:Start
	
	Start:
	
	mov   [Drive_ID], dl                                ; Save the boot drive ID --> MBR put it in DL
	mov   [DAP.Start_Sector], ebx                       ; Save the LBA of the start sector of the bootloader in the Disk Address Packet used by extended INT 0x13 

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
	mov   dl, BYTE [Drive_ID]
	mov   si, DAP
	mov   ah, 0x42
	int   0x13
	jnc   LaunchBootloader
	
	; If BIOS extensions do not exist or did not work, we will need to read from disk using INT 0x13, AH=0x02 that employs the CHS scheme
	
	DiskReadUsingCHS:

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
	jc    HaltSystem
	
	add   dh, 0x1                                       ; Since DH contains number of heads - 1, we add one to get the number of heads
	mov   [CHS_Geometry.Heads], dh                      ; Save the number of heads in memory
	and   cl, 0x3F                                      ; First 6 bits of CL contain the number of sectors per track
	mov   [CHS_Geometry.Sectors_Per_Track], cl          ; Save the number of sectors per track to memory
	mov   al, cl                                        ; number of sectors per cylinder = number of sectors per track x number of heads
	mul   dh
	mov   [CHS_Geometry.Sectors_Per_Cylinder], ax       ; Save the number of sectors per cylinder in memory
	
	; This is the code that converts the address of the starting sector from the LBA scheme to the CHS scheme 
	; It stores the C,H,S values in appropriate registers and then calls INT 0x13, AH=0x02
	; More details on the INT 0x13, AH=0x02 BIOS routine can be found in the MBR code

	mov   eax, DWORD [DAP.Start_Sector]
	mov   edx, 0
	movzx ebx, WORD [CHS_Geometry.Sectors_Per_Cylinder]
	div   ebx                                           ; LBA / sectors per cylinder = cylinder number
	cmp   eax, 0x3FF                                    ; cylinder number greater than 1023 is not supported by the BIOS INT 0x13, AH=0x02 routine
	jg    HaltSystem
	shl   ax, 0x6                            
	mov   cx, ax
	
	mov   ax, dx                                        ; DX contains LBA % sectors per cylinder
	div   BYTE [CHS_Geometry.Sectors_Per_Track]         ; (LBA % sectors per cylinder) % sectors per track = sector number (starting at 0)
	add   ah, 0x1                                       ; Sector number starts at 1
	and   ah, 0x3F                                      ; Sector numbers need to be in the range [1, 63]
	or    cl, ah                                        ; Store sector number in CL for INT 0x13, AH=0x02
	mov   dh, al                                        ; (LBA % sectors per cylinder) / sectors per track = head --> Store it in DH for INT 0x13, AH=0x02

	xor   ax, ax
	mov   es, ax
	mov   bx, Bootloader                                ; Store the load memory address in ES:BX
	mov   dl, BYTE [Drive_ID]                           ; Drive ID is stored in DL
	mov   al, BOOTLOADER_SIZE                           ; Number of sectors to copy are stored in AL

	; Now call INT 0x13, AH=0x02
	
	mov   ah, 0x02
	int   0x13
	jc    HaltSystem
	
	; We reach here if the disk read was successful 
	
	LaunchBootloader:
	mov   sp, STACK_TOP
	sub   sp, 0x6
	pop   si
	pop   ds
	pop   dx
	jmp   Bootloader
	
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
.Memory_Offset        dw Bootloader
.Memory_Segment       dw 0
.Start_Sector         dq 0

Messages:
.DiskIOErr            db 'Unable to read AVOS from disk', 0

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

Bootloader:
