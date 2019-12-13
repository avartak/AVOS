%include "boot/include/bootinfo.inc"                                    ; Common boot related information 

ORG 0

BITS 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MODLIST:

	jmp   Code
	nop
	
	BlockList:
	
	.Load_Address         dq 0x100000
	.Sector_Size          dw 0x200
	.Reserved             db 'KERNEL'
	
	.Block1_LBA           dq 0x800+PARTITION_START_LBA
	.Block1_Num_Sectors   dd 0x800
	
	times 0x200-($-$$)    db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	Code:

