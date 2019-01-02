; This is the code of the kernel of AVOS
; There is no code in this kernel. It only contains a C-style string of the message to the displayed on screen
; The kernel is to be placed at physical memory location 0x7E00 - so right after the bootloader
; In the floppy image as well the kernel come immediately after the boot sector

ORG 0x7C00

; We are still in real mode
BITS 16

; The first 512 bytes will be replaced in the disk image by the bootloader code
times 512 db 0

; Starting point of the kernel
Welcome_Message db 'Welcome to AVOS!', 0

; Adding a zero padding to the boot sector
times 1024-($-$$) db 0 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

