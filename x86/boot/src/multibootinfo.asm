; This code is taken from :
; https://wiki.osdev.org/Detecting_Memory_(x86)

; This is the function that reads the memory map using INT 0x15, AH=0xE820
; We are not going to try anything else right now if E820 fails

StoreMultibootInfo:

	push bp
	mov  bp, sp

    mov  bx, [bp+4]                                     ; Set the ES segment to the value of the first parameter passed to this function (expected to be 0x800)
    mov  es, bx
	mov  bx, [bp+6]
    mov  [es:bx], DWORD 8
	add  bx, 4
    mov  [es:bx], DWORD 0
	add  bx, 0x14
    mov  di, bx                                         ; Set DI to the address where the memory map boot information structure starts

    xor  ebx, ebx                                       ; EBX must be 0 to start
    xor  esi, esi                                       ; Clear the ESI register ; use for entry count
    mov  edx, 0x0534D4150                               ; Place "SMAP" into EDX
    mov  eax, 0xE820                                    ; Write the signature E820 into AX
    mov  [es:di+20], DWORD 1                            ; Require the entry to be valid for ACPI 3.0
    mov  ecx, 24                                        ; Ask for 24 bytes (what we get are potentially 20-bytes or 24-bytes)
    int  0x15
    jc   .retfalse                                      ; Function not supported if carry flag is set
    mov  edx, 0x0534D4150
    cmp  eax, edx                                       ; on success, EAX must have been reset to "SMAP"
    jne  .retfalse 
    test ebx, ebx                                       ; EBX = 0 implies list is only 1 entry long (worthless)
    je   .retfalse
    jmp  .checkentry

    .nextentry:
        test ebx, ebx                                   ; If EBX resets to 0, list is complete
        jz   .rettrue
        mov  eax, 0xE820                                ; EAX, ECX get trashed on every int 0x15 call
        mov  [es:di+20], DWORD 1                        ; Require the entry to be valid for ACPI 3.0
        mov  ecx, 24                                    ; Ask for 24 bytes again
        int  0x15
        jc  .rettrue                                    ; Carry set means "end of list already reached"
        mov  edx, 0x0534D4150                           ; Repair potentially trashed register

        .checkentry:
        jcxz .nextentry                                 ; Skip any 0 length entries
        cmp  cl, 20                                     ; Check for a 24-byte entry
        jbe  .addentry
        test byte [es:di+20], 1                         ; Got a 24 byte ACPI 3.X entry? Is the "ignore this data" bit clear?
        je   .nextentry

        .addentry:
        mov  ecx, [es:di+8]                             ; Get lower 4 bytes of memory region length
        or   ecx, [es:di+12]                            ; Do an OR with upper 4 bytes to test for zero
        jz   .nextentry                                 ; If the 8-byte value is 0, skip entry (overwrite it)
        add  esi, 24                                    ; Got a good entry: increment table size, move to next storage spot
        add  di, 24
        jmp  .nextentry

	.rettrue:
	mov  al, 1
	jmp  .end

	.retfalse:
	mov  al, 0

    .end:
    mov  bx, [bp+6]
	add  bx, 8
    add  esi, 0x10                                      ; There are 16 bytes of tag information in addition to the size
    mov  [es:bx],     DWORD 6                           ; Tag type
    mov  [es:bx+0x4], esi                               ; Size of the tag as a whole
    mov  [es:bx+0x8], DWORD 24                          ; Size of each E820 entry
    mov  [es:bx+0xC], DWORD 0                           ; E820 entry version (recommended to be 0)

    mov  bx, [bp+6]
    add  [es:bx], esi                                   ; Total size of all tags (+ this header)

    clc
    mov  di, [bp+6]
    add  bx, [es:di]
    add  [es:bx]  , DWORD 0                             ; Terminating tag 
    mov  [es:bx+4], DWORD 8

    xor  bx, bx
    mov  es, bx

	mov  sp, bp
	pop  bp
	ret
