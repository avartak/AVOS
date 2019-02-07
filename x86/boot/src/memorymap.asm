; This code is taken from :
; https://wiki.osdev.org/Detecting_Memory_(x86)

; This is the function that reads the memory map using INT 0x15, AH=0xE820
; We are not going to try anything else right now if E820 fails

StoreMemoryMap:
	push bp
	mov  bp, sp

	pusha
	push es

	mov  ax, [bp+4]                       ; Set the ES segment to the value of the first parameter passed to this function (expected to be 0x800)
	mov  es, ax 

	mov  di, [bp+6]                       ; Set DI to the second argument passed to the function. It's where the memory map boot information structure starts
	add  di, 0x10
	xor ebx, ebx                          ; EBX must be 0 to start
	xor esi, esi                          ; Clear the EBP register ; use for entry count
	mov edx, 0x0534D4150                  ; Place "SMAP" into edx
	mov eax, 0xE820                       ; Write the signature E820 into AX
	mov [es:di + 20], dword 1             ; Require the entry to be valid for ACPI 3.0
	mov ecx, 24                           ; Ask for 24 bytes (what we get are potentially 20-bytes or 24-bytes)
	int 0x15
	jc .failed                            ; carry set --> function not supported
	mov edx, 0x0534D4150
	cmp eax, edx                          ; on success, eax must have been reset to "SMAP"
	jne .failed
	test ebx, ebx		                  ; EBX = 0 implies list is only 1 entry long (worthless)
	je  .failed
	jmp .jmpin
.e820lp:
	mov eax, 0xe820                       ; EAX, ECX get trashed on every int 0x15 call
	mov [es:di + 20], dword 1             ; Require the entry to be valid for ACPI 3.0
	mov ecx, 24                           ; Ask for 24 bytes again
	int 0x15
	jc .e820f                             ; Carry set means "end of list already reached"
	mov edx, 0x0534D4150                  ; Repair potentially trashed register
.jmpin:
	jcxz .skipent                         ; Skip any 0 length entries
	cmp cl, 20                            ; Got a 24 byte ACPI 3.X response?
	jbe short .notext
	test byte [es:di + 20], 1             ; if so: is the "ignore this data" bit clear?
	je .skipent
.notext:
	mov ecx, [es:di + 8]	              ; get lower uint32_t of memory region length
	or  ecx, [es:di + 12]	              ; "or" it with upper uint32_t to test for zero
	jz .skipent                           ; If length uint64_t is 0, skip entry
	add si, 24                            ; Got a good entry: increment table size, move to next storage spot
	add di, 24
.skipent:
	test ebx, ebx                         ; If EBX resets to 0, list is complete
	jne .e820lp
.e820f:
	mov bx, [bp+6]
	add esi, 0x10                         ; There are 16 bytes of tag information in addition to the size
	mov [es:bx],     DWORD 6              ; Tag type
	mov [es:bx+0x4], esi                  ; Size of the tag as a whole
	mov [es:bx+0x8], DWORD 24             ; Size of each E820 entry
	mov [es:bx+0xC], DWORD 0              ; E820 entry version (recommended to be 0)

	mov bx,  [bp+8]
	add [es:bx], esi                      ; Total size of all tags (+ this header)

	clc			                          ; There is "jc" on end of list to this point, so the carry must be cleared

	pop es
	popa
	mov sp, bp
	pop bp
	ret
.failed:
	stc                                  ; "Function unsupported" error exit

	pop es
	popa
	mov sp, bp
	pop bp
	ret
