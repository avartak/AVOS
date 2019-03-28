LoadGDT:

	; Global Descriptor Table
	; 8-byte entries for each segemnt type
	; First entry is default, contains all zeroes
	; For details on segment descriptors see : https://wiki.osdev.org/Global_Descriptor_Table
	; This is a temporary location for the table
	; It should be moved to a more permanent place by the kernel
	; For now we just use this for setting up the protected mode
	
	push bp
	mov  bp, [bp+0x4]
	
	; First there is a NULL segment
	mov [bp+0x0],  WORD 0x0000
	mov [bp+0x2],  WORD 0x0000
	mov [bp+0x4],  BYTE 0x00
	mov [bp+0x5],  BYTE 0x00
	mov [bp+0x6],  BYTE 0x00
	mov [bp+0x7],  BYTE 0x00
	
	; Then we insert the kernel code segment
	mov [bp+0x8] ,  WORD 0xFFFF
	mov [bp+0xA] ,  WORD 0x0000
	mov [bp+0xC],   BYTE 0x00
	mov [bp+0xD],   BYTE 0x9A
	mov [bp+0xE],   BYTE 0xCF
	mov [bp+0xF],   BYTE 0x00
	
	; Next we insert the kernel data segment
	mov [bp+0x10],  WORD 0xFFFF
	mov [bp+0x12],  WORD 0x0000
	mov [bp+0x14],  BYTE 0x00
	mov [bp+0x15],  BYTE 0x92
	mov [bp+0x16],  BYTE 0xCF
	mov [bp+0x17],  BYTE 0x00
	
	; GDT descriptor
	mov [bp+0x18],  WORD 0x18 - 1
	mov [bp+0x1A],  WORD bp
	mov [bp+0x1C],  WORD 0
	mov [bp+0x1E],  WORD 0
	
	; Load the GDT
	lgdt [bp+0x18]
	
	pop  bp
	ret
