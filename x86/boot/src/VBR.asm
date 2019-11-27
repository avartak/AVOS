; The Volume Boot Record (VBR) is the first sector of the 'active' or bootable partition that contains our OS
; The Master Boot Record (MBR) first identifies the active partition, then loads the VBR at memory address 0x7C00, and transfers control to it
; The VBR is the first OS-specific code
; Salient features of our VBR :
; - Starts with JMP to the VBR code
; - There are 124 bytes left for OS-specific (or filesystem specific) data/header after the first two bytes of the JMP instruction + 2 NOPs (the JMP jumps over this block) 
;   * We use these 124 bytes to store a blocklist of sectors containing the bootload code on disk
;   * Each blocklist entry contains the offset of the starting sector relative to the start of the partition, and the number of contiguous sectors to read out
; - The VBR also has the boot signature word (0xAA55) at its end, just like the MBR
; - The VBR then loads the bootloader code from disk to memory
; - The VBR saves (in addition to DL, ES:DI and DS:SI values passed down by the MBR) : 
;   * The memory address of a 16-byte partition entry (8-bytes for starting LBA and 8-bytes for ending LBA) in FS:BP
; - Note the VBR interface provides 64-bit LBA addresses for the partition boundaries to the bootloader
; - This VBR only supports 32-bit LBAs (since it reads them from the MBR partition table)
; - As long as the VBR interface is maintained, some different code could also pass the partition boundaries from the GPT (16-bytes at offset 0x20 in the GPT partition entry)

; First let us include some definitions of constants that the VBR needs

SECTOR_SIZE             equ 0x0200                         ; Assumed size of a sector
VBR_ADDRESS             equ 0x7C00                         ; This is where the VBR is loaded in memory
STACK_TOP               equ 0x7C00                         ; Top of the stack used by the MBR
BOOTLOADER_ADDRESS      equ 0x7E00                         ; Starting location in memory where the bootloader code gets loaded

; We need to tell the assembler that all labels need to be resolved relative to the memory address 0x7C00 in the binary code

ORG VBR_ADDRESS

; The x86 system is still in real mode when control is transferred to the VBR
; We need to tell the assembler to produce 16-bit code

BITS 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

VBR:

	jmp   Code
	nop
	nop

	BlockList:

	; We reserve the next 124 bytes for the blocklist corresponding to the bootloader code on disk
	; This is basically a table containing (up to) 8 twelve-byte entries + 1 null entry (zero size)
	; The first 8 bytes of the blocklist contain the 64-bit memory address from where to start loading the code in memory
	; Next 8-bytes are reserved
	; Then come the blocklist entries
	; First 8 bytes of each entry contain the 64-bit LBA offset (w.r.t. the partition) of the start sector of a 'block' containing the bootloader code
	; The last 4 bytes of each entry contain the size of the block (number of contiguous sectors to be read out)
	; An entry with 0 size marks the end of the blocklist, all remaining entries will be ignored

	; Note : 
	; The VBR does not check the size of a sector (whether it is 512 bytes, or 4 KB, etc.)
	; A default size of 512 bytes is assumed 
	; But this can be changed by setting the word at offset 0xC from the start of the VBR when install the bootloader (e.g. when creating the blocklist)

	.Load_Address         dq BOOTLOADER_ADDRESS
	.Sector_Size          dw SECTOR_SIZE
	.Reserved1            dw 0
	.Reserved2            dd 0

	.Block1_LBA           dq 8
	.Block1_Num_Sectors   dd 0x40

	; Pad the remaining bytes up to VBR+128 with zero -- 128 = 124 (blocklist) + 4 (JMP 2 bytes + 2 NOPs)

	times 128-($-$$)      db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	; This is where the code starts -- VBR + 128

	Code:

	; Expect the boot drive ID in DL and the active partition table entry in the relocated MBR in DS:SI when control is transferred to the boot loader. 
	; ES:DI may point to "$PnP" installation check structure for systems with Plug-and-Play BIOS or BBS support
	; Save these registers on the stack

	; First lets set up the stack properly

	xor   ax, ax
	mov   ss, ax

	; Lets make certain that CS is set to 0x0000
	
	jmp   0x0000:Start

	; Now we are ready to launch into the bootloading

	Start:

	mov   sp, STACK_TOP
	push  dx
	push  es
	push  di
	push  ds
	push  si

	; Save the LBA (low DWORD) of the start sector of this partition from the MBR partition table [restricts us to 32-bit LBA]
	
	mov   ebp, DWORD [si+0x08]

	; Lets initialize the segment registers to the base address values that we want to use (0x0000)
	
	xor   ax, ax
	mov   ds, ax
	mov   es, ax

	; Check for BIOS extensions to read from disk using the LBA scheme

	mov   bx, 0x55AA
	mov   ah, 0x41
	int   0x13
	jc    HaltSystem
	cmp   bx, 0xAA55
	jne   HaltSystem
	test  cx, 1
	jz    HaltSystem

	; Save the starting address of the bootloader blocklist entries
	
	mov   di, BlockList.Block1_LBA
	
	; Loop on the blocklist to load the bootloader code from disk to memory

	ReadLoop:
	cmp   DWORD [di+8], 0                                  ; Check the number of sectors to be read out is 0 --> indicates the end of the block list
	je    LaunchBootloader

	DiskRead:
	mov   ebx, [di]
	mov   ecx, [di+4]

	xor   eax, eax
	mov   al, 0x7F
	cmp   [di+8], ax
	jg    PrepareRead
	mov   ax, [di+8]
	
	PrepareRead:                                           ; Save the necessary information needed by (extended) BIOS routines to read from disk --> Fill the DAP
	sub   [di+8], ax
	
	add   [di], eax
	adc   DWORD [di+4], 0

	mov   [DAP.Sectors_Count], al
	mov   [DAP.Start_Sector], ebp
	add   [DAP.Start_Sector], ebx
	adc   [DAP.Start_Sector+4], ecx
	
	DiskReadUsingLBA:
	mov   dl, [STACK_TOP-2]
	mov   si, DAP
	mov   ah, 0x42
	int   0x13
	jnc   ReadLoopCheck
	jmp   HaltSystem
	
	; Check if we still need to loop

	ReadLoopCheck:
	mov   bx, [DAP.Memory_Segment]
	shl   ebx, 4
	add   ebx, [DAP.Memory_Offset]
	mov   eax, [DAP.Sectors_Count]
	movzx ecx, WORD [BlockList.Sector_Size]
	mul   ecx
	add   eax, ebx
	mov   ax, 0x000F
	mov   [DAP.Memory_Offset], ax
	shr   eax, 4
	mov   [DAP.Memory_Segment], ax
	cmp   DWORD [di+8], 0
	jne   DiskRead
	add   di, 10
	jmp   ReadLoop
	
	; We reach here if the disk read was successful 
	
	LaunchBootloader:
	mov   sp, STACK_TOP-10
	pop   si
	pop   ds
	pop   di
	pop   es
	pop   dx

	mov   eax, DWORD [si+0x08]                             ; Get the LBA (low DWORD) of the start sector of this partition from the MBR partition table [restricts us to 32-bit LBA]
	mov   ecx, DWORD [si+0x0C]                             ; Get the size of the partition in sectors from the MBR partition table [restricts us to 32-bits]
	mov   DWORD [Partition], eax                           ; Save the lower 4 bytes of the 64-bit LBA of the start sector of the partition
	mov   DWORD [Partition+8], eax                         ; Save the 64-bit LBA of the end sector of the partition (start + size)
	add   DWORD [Partition+8], ecx
	adc   DWORD [Partition+0x10], 0
	mov   bp, Partition
	xor   ax, ax
	mov   fs, ax

	mov   ax, [BlockList+2]
	push  ax
	mov   ax, [BlockList]
	push  ax
	retf
	
	; If we did not read what we wanted to we halt
	; Before halting we print an error message on the screen 
	
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

	; Some data/information we need

	DAP:
	.Size                 db 0x10
	.Unused1              db 0
	.Sectors_Count        db 0
	.Unused2              db 0
	.Memory_Offset        dw BOOTLOADER_ADDRESS
	.Memory_Segment       dw 0
	.Start_Sector         dq 0
	
	Messages:
	.DiskIOErr            db 'Unable to load AVOS', 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Padding of zeroes till offset 446 : location of the VBR partition table (if any)
; We put a 16-byte entry at offset 446 corresponding to the 64-bit LBAs of the start and end sectors of the partition
; The address of this 16-byte entry is passed to the next step of the bootloader in the FS:BP registers

	times 446-($-$$) db 0

	Partition:
	times 16              db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Padding of zeroes till the end of the boot sector (barring the last two bytes that are reserved for the boot signature)

times 510-($-$$) db 0 

; The last two bytes need to have the following boot signature -- MBR code typically checks for it

Boot_Signature:
dw   0xAA55

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


