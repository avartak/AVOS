CreateGDT:

    ; Global Descriptor Table
    ; 8-byte entries for each segemnt type
    ; First entry is default, contains all zeroes
    ; For details on segment descriptors see : https://wiki.osdev.org/Global_Descriptor_Table
    ; This is a temporary location for the table
    ; It should be moved to a more permanent place by the kernel
    ; For now we just use this for setting up the protected mode

    GDT_Start  equ START_GDT
    GDT_Null   equ GDT_Start
    GDT_KCode  equ GDT_Null   + 8
    GDT_KData  equ GDT_KCode  + 8
    GDT_End    equ GDT_KData  + 8
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

    mov [GDT_Desc  +0],  WORD GDT_End - GDT_Start - 1
    mov [GDT_Desc  +2], DWORD GDT_Start
    mov [GDT_Desc  +6],  WORD 0


	ret
