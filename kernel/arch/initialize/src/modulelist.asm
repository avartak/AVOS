; Common boot related information 

ORG 0

BITS 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MODLIST:

	jmp   Code
	nop
	
	BlockList:
	
	.Address              dq 0
	.Sector_Size          dw 0x200
	.Reserved             db 'KERNEL'
	
	.Block1_LBA           dq 0x800+0x800
	.Block1_Num_Sectors   dd 0x800
	
	times 0x200-($-$$)    db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	Code:

