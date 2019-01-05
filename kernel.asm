; This is the kernel of AVOS sitting at the physical address of 1 MB

[ORG 0x100000]

; We are now firmly in protected mode
[BITS 32]

; Starting point of the kernel

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Kernel:


	mov cl, 0x04
	mov dl, 0

	call Clear_Screen
	inc dl
	inc dl
	mov esi, Welcome_Message_Ln1
	call Print
	inc dl
	mov esi, Welcome_Message_Ln2
	call Print
	inc dl
	mov esi, Welcome_Message_Ln3
	call Print
	inc dl
	mov esi, Welcome_Message_Ln4
	call Print
	inc dl
	mov esi, Welcome_Message_Ln5
	call Print
	inc dl
	mov esi, Welcome_Message_Ln6
	call Print

	hlt                                       ; Halt the system	

%include "print.asm"




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; Welcome message for the OS
Welcome_Message_Ln1 db '                           ___  _   _ _____ _____   _ ', 0
Welcome_Message_Ln2 db '                          / _ \| | | |  _  /  ___| | |', 0
Welcome_Message_Ln3 db '                         / /_\ \ | | | | | \ `--.  | |', 0
Welcome_Message_Ln4 db '                         |  _  | | | | | | |`--. \ | |', 0
Welcome_Message_Ln5 db '                         | | | \ \_/ | \_/ /\__/ / |_|', 0
Welcome_Message_Ln6 db '                         \_| |_/\___/ \___/\____/  (_)', 0
                                                                                       


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

