; This code is taken from :
; https://wiki.osdev.org/Detecting_Memory_(x86)

; First let us include some definitions of constants (the constants themselves are described in comments)

%include "x86/boot/src/defs.asm"

; This is the function that reads the memory map using INT 0x15, AH=0xE820
; We are not going to try anything else right now if E820 fails

StoreMemoryMap:
	pusha
	push es

	mov  ax, 0x800                      ; Set the ES segment to 0x800. This means the starting point in physical memory is 0x800*0x10 + 0x8000 = 0x10000 
	mov  ds, ax   
	mov  es, ax 

	mov di, 0x8004                      ; Set DI to 0x8004. Otherwise this code will get stuck in `int 0x15` after some entries are fetched 
	xor ebx, ebx                        ; EBX must be 0 to start
	xor bp, bp                          ; keep an entry count in BP
	mov edx, 0x0534D4150                ; Place "SMAP" into edx
	mov eax, 0xE820                     ; Write the signature E820 into AX
	mov [es:di + 20], dword 1           ; Require the entry to be valid for ACPI 3.0
	mov ecx, 24                         ; Ask for 24 bytes (what we get are potentially 20-bytes or 24-bytes)
	int 0x15
	jc short .failed                    ; carry set --> function not supported
	mov edx, 0x0534D4150
	cmp eax, edx                        ; on success, eax must have been reset to "SMAP"
	jne short .failed
	test ebx, ebx		                ; EBX = 0 implies list is only 1 entry long (worthless)
	je short .failed
	jmp short .jmpin
.e820lp:
	mov eax, 0xe820                     ; EAX, ECX get trashed on every int 0x15 call
	mov [es:di + 20], dword 1           ; Require the entry to be valid for ACPI 3.0
	mov ecx, 24                         ; Ask for 24 bytes again
	int 0x15
	jc short .e820f                     ; Carry set means "end of list already reached"
	mov edx, 0x0534D4150                ; Repair potentially trashed register
.jmpin:
	jcxz .skipent                       ; Skip any 0 length entries
	cmp cl, 20                          ; Got a 24 byte ACPI 3.X response?
	jbe short .notext
	test byte [es:di + 20], 1           ; if so: is the "ignore this data" bit clear?
	je short .skipent
.notext:
	mov ecx, [es:di + 8]	            ; get lower uint32_t of memory region length
	or ecx, [es:di + 12]	            ; "or" it with upper uint32_t to test for zero
	jz .skipent                         ; If length uint64_t is 0, skip entry
	inc bp			                    ; Got a good entry: ++count, move to next storage spot
	add di, 24
.skipent:
	test ebx, ebx                       ; If EBX resets to 0, list is complete
	jne short .e820lp
.e820f:
	mov [es:START_MEM_MAP], bp          ; Store the entry count
	clc			                        ; There is "jc" on end of list to this point, so the carry must be cleared

	pop ds
	popa
	ret
.failed:
	stc                                 ; "Function unsupported" error exit

	pop ds
	popa
	ret
