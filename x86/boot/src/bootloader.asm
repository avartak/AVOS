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
	; Instead the code in the boot sector will simply load 8 KB from the floppy drive (specifically, logical address of 4 KB - 12 KB on the floppy drive) to the memory location 0x8000
	; This is meant to be the 2nd stage of the boot process. It's a loader that will set up the 32-bit system (protected mode) and load the 3rd stage 
	; The 3rd stage is an 8 KB, 32-bit code that will set up paging, load the kernel at 1 MB physical address, and map it to 3 GB virtual address, and then launch the kernel
	; The third stage is copied from logical address 12 KB - 20 KB of the floppy drive to memory location 0xA000
	; The kernel is assumed to be 1 MB of code starting at 20 KB logical address on the floppy

	; For now the three stages of the bootstrap will only perform the minimal tasks highlighted above and launch the kernel
	; There remains a lot of room for running a more sophisticated boot in the 16-bit and 32-bit environments if needed -- assuming 8 KB is sufficient for that purpose
	; Take note of the mapping of the first MB of the physical memory in the PC architecture. 

	; 0x000000 - 0x000400 : Interrupt Vector Table  
	; 0x000400 - 0x000500 : BIOS data area          
	; 0x000500 - 0x007C00 : Free area         
	; 0x007C00 - 0x007E00 : Boot sector             
	; 0x007E00 - 0x09FC00 : Free area               
	; 0x09FC00 - 0x0A0000 : Extended bios data area 
	; 0x0A0000 - 0x0C0000 : Video memory            
	; 0x0C0000 - 0x100000 : BIOS            

	; As mentioned above our three stages of bootstrap will reside in :
	; 0x007C00 - 0x007E00
	; 0x008000 - 0x00A000
	; 0x00A000 - 0x00C000

	; There is an 8 KB stack set up at :
	; 0x00C000 - 0x00E000

	; There is some scratch space of 4 KB
	; 0x00E000 - 0x00F000

	; There is 4 KB space for the GDT and IDT
	; 0x00F000 - 0x010000

	; Finally, we will need some space (8 KB to be precise) to put the paging tables required to set up the higher half kernel
	; 0x10000  - 0x12000

	; So, lets set up the segment registers -- DS, ES at 0, and SS at 0xC00, and the stack pointer (SP) at 0x2000

	mov ax, SEG_DS16
	mov ds, ax
	mov ax, SEG_ES16
	mov es, ax
	mov ax, SEG_SS16
	mov ss, ax
	mov sp, SIZE_STACK

	; This is the code that reads the 2nd (16-bit) stage of the boot loader
	; As of now, it reads a certain number of sectors starting from a given location on a floppy disk
	; In principle it should work for hard disk drives as well if we set the drive ID appropriately, although that has not been tested
	; The drive ID is passed as a parameter in register DL (0 for floppy disk, 0x80 for HDD -- not tried)
	; Next, we obtain the parameters of the drive that we want to read from
	; We then read N sectors starting from sector S and these will be placed in memory contiguously from ES:SI
	; For this we will invoke the ReadSectorsFromDrive function
	; Parameter S is stored in AX, N is stored in BL, drive ID is stored in DL
	; When we have our own file system this should be expanded to read a file from stored on the drive

	mov  dl, FLOPPY_ID                        ; Pass the Drive ID parameter 
	call ReadDriveParameters

	mov dl, FLOPPY_ID                         ; Pass the Drive ID parameter 
	mov ax, START_BOOT2_DISK                  ; Copy data starting from 4 KB logical address of the disk
	mov bl, SIZE_BOOT2_DISK                   ; Copy 8 KB of data (16 sectors)
	mov si, START_BOOT2                       ; LoadKernel function will copy the kernel image from disk to ES:SI
	call ReadSectorsFromDrive

	mov al, [Sectors_Read_Last]
	cmp al, bl
	je .launchstage2
	hlt

	.launchstage2:
	jmp 0x0:START_BOOT2 


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "x86/boot/src/biosio.asm"            ; ReadDriveParameters and ReadSectorsFromDrive are define in this file

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Adding a zero padding to the boot sector

times SIZE_BOOT1-2-($-$$) db 0 

; The last two bytes of the boot sector need to have the following boot signature 
; Otherwise BIOS will not recognize this as a boot sector

dw BOOTSECT_MAGIC

