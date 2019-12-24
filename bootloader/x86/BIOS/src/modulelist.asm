; Common boot related information 

%include "bootloader/x86/BIOS/include/bootinfo.inc"

ORG 0

BITS 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MODLIST:

	jmp   Code
	nop
	
	BlockList:
	
	.Address              dq 0
	.Sector_Size          dw SECTOR_SIZE
	.Reserved             db 'KERNEL'
	
	.Block1_LBA           dq 0x800+PARTITION_START_LBA
	.Block1_Num_Sectors   dd 0x800
	
	times 0x200-($-$$)    db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	Code:

