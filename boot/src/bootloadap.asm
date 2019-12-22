
%include "boot/include/bootinfo.inc"                                    ; Common boot related information 

ORG BOOTLOADAP_ADDRESS

BITS 16

Start:

	hlt
	jmp   Start

times BOOTLOADER_ADDRESS-BOOTLOADAP_ADDRESS-($-$$) db 0

