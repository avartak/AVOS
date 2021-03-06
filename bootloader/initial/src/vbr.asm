; The Volume Boot Record (VBR) is the first sector of the 'active' or bootable partition that contains our OS
; The Master Boot Record (MBR) first identifies the active partition, then loads the VBR at memory address 0x7C00, and transfers control to it
; The VBR is the first OS-specific code, it loads the bootloader code from disk to memory
; Salient features of our VBR :
; - Starts with JMP over the first 128 bytes of data to the VBR code
; - First 128 bytes : First two bytes for a near JMP + 2 NOPs (the JMP jumps over this block) ; then 124 bytes for OS-specific (or filesystem specific) data/header
;   * We use these 124 bytes to store a blocklist of sectors containing the bootload code on disk
;   * Each blocklist entry contains the LBA of the start sector of the partition, and the number of contiguous sectors to be read out
; - The VBR also has the boot signature word (0xAA55) at its end, just like the MBR
; - The VBR preserves the DL, ES:DI and DS:SI values passed down by the MBR

; First let us include some definitions of constants that the VBR needs

%include "bootloader/initial/include/bootinfo.inc"         ; Common boot related information 

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
	; First 8 bytes of each entry contain the 64-bit LBA of the start sector of a 'block' containing the bootloader code
	; The last 4 bytes of each entry contain the size of the block (number of contiguous sectors to be read out)
	; An entry with 0 size marks the end of the blocklist, all remaining entries will be ignored

	; Note : 
	; The VBR gets the size of a sector (whether it is 512 bytes, or 4 KB, etc.) from BIOS and compares it with the value stored in the blocklist to make sure they agree

	.Address              dq BOOTLOADER_ADDRESS
	.Sector_Size          dw SECTOR_SIZE
	.Reserved1            dw 0
	.Reserved2            dd 0

	.Block1_LBA           dq 0x8+PARTITION_START_LBA
	.Block1_Num_Sectors   dd 0x40
	.Block_Size:          equ $-BlockList.Block1_LBA

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

	; Lets initialize the segment registers to the base address values that we want to use (0x0000)
	
	mov   ds, ax
	mov   es, ax

	; Check for BIOS extensions to read from disk using the LBA scheme

	mov   bx, 0x55AA
	mov   ah, 0x41
	int   0x13
	jc    HaltSystem
	cmp   bx, 0xAA55
	jne   HaltSystem
	test  cl, 1
	jz    HaltSystem

	; Save the disk geometry, and check that the sector size is the same as that specified in the blocklist

	mov   si, DGP
	mov   ah, 0x48
	int   0x13
	jc    HaltSystem
	mov   ax, [DGP.Bytes_Per_Sector]
	cmp   ax, [BlockList.Sector_Size]
	jne   HaltSystem

	; Save the starting address of the bootloader blocklist entries 
	
	mov   di, BlockList.Block1_LBA

	; Loop on the blocklist to load the bootloader code from disk to memory

	ReadLoop:
	cmp   di, Code                                         ; Check that we are not at the end of the 128 bytes of the blocklist 
	jge   HaltSystem
	cmp   DWORD [di+8], 0                                  ; Check the number of sectors to be read out is 0 --> indicates the end of the block list
	je    LaunchBootloader

	DiskRead:
	xor   eax, eax
	mov   al, 0x7F                                         ; Maximum number of sectors that some BIOSes (e.g. Phoenix BIOS) will read with INT 0x13, AH=0x42
	cmp   [di+8], eax
	jg    PrepareRead
	mov   eax, [di+8]
	
	PrepareRead:                                           ; Save the necessary information needed by (extended) BIOS routines to read from disk --> Fill the DAP
	mov   ebx, [di]
	mov   ecx, [di+4]

	sub   [di+8], eax
	add   [di], eax
	adc   DWORD [di+4], 0

	mov   [DAP.Sectors_Count], al
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
	movzx ebx, WORD [DAP.Memory_Segment]
	shl   ebx, 4
	add   ebx, [DAP.Memory_Offset]
	movzx eax, BYTE [DAP.Sectors_Count]
	movzx ecx, WORD [BlockList.Sector_Size]
	mul   ecx
	add   eax, ebx
	mov   [DAP.Memory_Offset], ax
	shr   eax, 0x10
	shl   eax, 0xC
	mov   [DAP.Memory_Segment], ax
	cmp   DWORD [di+8], 0
	jne   DiskRead
	add   di, BlockList.Block_Size
	jmp   ReadLoop
	
	; We reach here if the disk read was successful 
	
	LaunchBootloader:
	mov   sp, STACK_TOP-10
	pop   si
	pop   ds
	pop   di
	pop   es
	pop   dx

	push  DWORD [BlockList]
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

; Inputs/variables needed by the VBR code

	DAP:
	.Size                 db DGP-DAP
	.Unused1              db 0            
	.Sectors_Count        db 1            
	.Unused2              db 0            
	.Memory_Offset        dw BOOTLOADER_ADDRESS
	.Memory_Segment       dw 0
	.Start_Sector         dq 0

	DGP:
	.Size                 dw DGP.DPTE_Ptr-DGP
	.Flags                dw 0
	.Cylinders            dd 0
	.Heads                dd 0
	.Sectors_Per_Track    dd 0
	.Sectors              dq 0
	.Bytes_Per_Sector     dw 0
	.DPTE_Ptr             dd 0

	Messages:
	.DiskIOErr            db 'Boot error', 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Padding of zeroes till the end of the boot sector (barring the last two bytes that are reserved for the boot signature)

times VBR_SIZE-2-($-$$)   db 0 

; The last two bytes need to have the following boot signature -- MBR code typically checks for it

Boot_Signature            dw 0xAA55

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


