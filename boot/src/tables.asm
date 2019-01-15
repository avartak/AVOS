    ; Interrupt Descriptor Table
    ; We are not enabling interrupts here
    ; This is just a place reserved for the IDT
    ; The IDT is basically an array of 256 interrupt descriptors each 64 bit (2 bytes) in size
    ; Therefore, the size of the IDT is 2 KB
    ; We will simply put 2 KB of zeroes for the IDT

	; The START_DESCR and SIZE_IDT constants have been defined in defs.asm -- already included in kload16.asm, so not included here

    IDT_Start equ START_DESCR
    IDT_End   equ IDT_Start + SIZE_IDT

    mov cx, SIZE_IDT
    .fillIDT :
        dec cx
        mov di, IDT_Start
        add di, cx
        mov al, 0
        stosb
        cmp cx, 0
        jne .fillIDT
   

    ; Global Descriptor Table
    ; 8-byte entries for each segemnt type
    ; First entry is default, contains all zeroes
    ; For details on segment descriptors see : https://wiki.osdev.org/Global_Descriptor_Table
    ; This is a temporary location for the table
    ; It should be moved to a more permanent place by the kernel
    ; For now we just use this for setting up the protected mode

    GDT_Start  equ IDT_End
    GDT_Null   equ GDT_Start
    GDT_KCode  equ GDT_Null   + 8
    GDT_KData  equ GDT_KCode  + 8
    GDT_KStack equ GDT_KData  + 8
    GDT_UCode  equ GDT_KStack + 8
    GDT_UData  equ GDT_UCode  + 8
    GDT_UStack equ GDT_UData  + 8
    GDT_End    equ GDT_UStack + 8
    GDT_Desc   equ GDT_End

    ; First there is a NULL segment
    mov [GDT_Null  +0],  WORD 0x0000
    mov [GDT_Null  +2],  WORD 0x0000
    mov [GDT_Null  +4],  BYTE 0x00
    mov [GDT_Null  +5],  BYTE 0x00
    mov [GDT_Null  +6],  BYTE 0x00
    mov [GDT_Null  +7],  BYTE 0x00

    ; Then we insert the kernel code segment
    mov [GDT_KCode +0],  WORD 0xFFFF
    mov [GDT_KCode +2],  WORD 0x0000
    mov [GDT_KCode +4],  BYTE 0x00
    mov [GDT_KCode +5],  BYTE 0x9A
    mov [GDT_KCode +6],  BYTE 0xCF
    mov [GDT_KCode +7],  BYTE 0x00

    ; Next we insert the kernel data segment
    mov [GDT_KData +0],  WORD 0xFFFF
    mov [GDT_KData +2],  WORD 0x0000
    mov [GDT_KData +4],  BYTE 0x00
    mov [GDT_KData +5],  BYTE 0x92
    mov [GDT_KData +6],  BYTE 0xCF
    mov [GDT_KData +7],  BYTE 0x00

    ; And then the kernel stack segment
    mov [GDT_KStack+0],  WORD 0xFFFF
    mov [GDT_KStack+2],  WORD 0x0000
    mov [GDT_KStack+4],  BYTE 0x00
    mov [GDT_KStack+5],  BYTE 0x96
    mov [GDT_KStack+6],  BYTE 0xCF
    mov [GDT_KStack+7],  BYTE 0x00

    ; Next user code segment
    mov [GDT_UCode +0],  WORD 0xFFFF
    mov [GDT_UCode +2],  WORD 0x0000
    mov [GDT_UCode +4],  BYTE 0x00
    mov [GDT_UCode +5],  BYTE 0xFA
    mov [GDT_UCode +6],  BYTE 0xCF
    mov [GDT_UCode +7],  BYTE 0x00

    ; Followed by user data segment
    mov [GDT_UData +0],  WORD 0xFFFF
    mov [GDT_UData +2],  WORD 0x0000
    mov [GDT_UData +4],  BYTE 0x00
    mov [GDT_UData +5],  BYTE 0xF2
    mov [GDT_UData +6],  BYTE 0xCF
    mov [GDT_UData +7],  BYTE 0x00

    ; And finally the user stack segment
    mov [GDT_UStack+0],  WORD 0xFFFF
    mov [GDT_UStack+2],  WORD 0x0000
    mov [GDT_UStack+4],  BYTE 0x00
    mov [GDT_UStack+5],  BYTE 0xF6
    mov [GDT_UStack+6],  BYTE 0xCF
    mov [GDT_UStack+7],  BYTE 0x00

    mov [GDT_Desc  +0],  WORD GDT_End - GDT_Start - 1
    mov [GDT_Desc  +2], DWORD GDT_Start
    mov [GDT_Desc  +6],  WORD 0

