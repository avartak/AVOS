; This is the code of the 16-bit (REAL mode) part of the bootloader
; We are still in REAL mode
; This code makes the switch to PROTECTED mode and transfers control to the 32-bit boot loader that will then copy the kernel to memory

; Some notes about the mapping of the first MB of the physical memory in the PC architecture, and how we use this memory 

; 0x000000 - 0x000400 : Interrupt Vector Table  
; 0x000400 - 0x000500 : BIOS data area          
; 0x000500 - 0x007C00 : Usable
; 0x000600 - 0x000800 : Relocated MBR (It's stack top is at 0x0600)
; 0x000800 - 0x007C00 : Free
; 0x007C00 - 0x007E00 : VBR (It's stack top is at 0x7C00)
; 0x007E00 - 0x09FC00 : Free
; 0x007E00 - 0x008000 : 16-bit part of the boot loader code (It's stack top is at 0x7C00)
; 0x008000 - 0x00FFFF : 32-bit part of the boot loader code (It's stack top is at 0x7C00)    
; 0x09FC00 - 0x0A0000 : Extended BIOS data area 
; 0x0A0000 - 0x0C0000 : Video memory            
; 0x0C0000 - 0x100000 : BIOS            

; First let us include some definitions of constants

STACK_TOP               equ 0x7C00                                      ; Top of the stack
SCREEN_TEXT_BUFFER      equ 0xB800                                      ; Video buffer for the 80x25 VBE text mode (for displaying error messages)
BOOTLOADER_ADDRESS      equ 0x7E00                                      ; Starting location in memory where the bootloader code gets loaded
SEG32_CODE              equ 0x08                                        ; 32-bit kernel code segment
SEG32_DATA              equ 0x10                                        ; 32-bit kernel data segment

; Starting point of the bootloader in memory --> follows immediately after the 512 bytes of the VBR

ORG BOOTLOADER_ADDRESS

; We are still in 16-bit real mode

BITS 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

AVBL16:

	jmp   Code
	nop
	nop

    BlockList:

    ; We reserve the next 124 bytes for the blocklist corresponding to the kernel code on disk
    ; This is basically a table containing (up to) 12 ten-byte entries
    ; The first 4 bytes of the blocklist contain the 32-bit start address where the kernel is loaded in memory
    ; Then come the blocklist entries
    ; First 8 bytes of each entry contain the 64-bit LBA offset (w.r.t. the partition) of the start sector of a 'block' containing the code
    ; The last 2 bytes contain the size of the block (number of contiguous sectors to be read out)
    ; An entry with 0 size marks the end of the blocklist, all remaining entries will be ignored

    .Load_Offset          dw BOOTLOADER_ADDRESS
    .Load_Segment         dw 0

    .Block1_LBA           dq 0x800
    .Block1_Num_Sectors   dw 0x800

	times 124+4-($-$$)    db 0                                          ; The 4 accounts for the 2 bytes taken up by the JMP instruction + 2 NOPs

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	; This is where the code starts

	Code:

	; Expect the stack to be appropriately set up when control is handed over here
	; Expect the boot drive ID in DL and the active partition table entry in the relocated MBR in DS:SI when control is transferred to the boot loader. 
	; ES:DI may point to "$PnP" installation check structure for systems with Plug-and-Play BIOS or BBS support
    ; Save these registers on the stack

	push  dx
	push  es
	push  di
	push  ds
	push  si
	push  fs
	push  bp

	; Set all the segment registers to the base address we want (0x0000)
	
	xor   ax, ax
	mov   ds, ax	
	mov   es, ax	
	jmp   0x0000:Start

	; To enable the protected mode we will :
	; - Switch on the A20 line 
	; - Create a GDT and load it
	; - Set the first bit in CR0 to 1
	
	Start:

	; Enable the A20 line

	call  A20_Enable
	test  al, al
	jz    HaltSystem

	; Save the boot drive ID in DL, the location of MBR's active partition in ESI, and information about "$PnP" installation check structure in EDI
	; Save the pointer to the 16-bytes containing the 64-bit start and end LBAs of this partition in EBP
	; Save the pointer to the kernel code blocklist in EBX

	xor   eax, eax
	xor   ebp, ebp
	pop   ax
	pop   bp
	shl   ebp, 4
	add   ebp, eax

	xor   esi, esi
	pop   ax
	pop   si
	shl   esi, 4
	add   esi, eax

	xor   edi, edi
	pop   ax
	pop   di
	shl   edi, 4
	add   edi, eax

	mov   ebx, BlockList

	pop   dx

	; Set up the switch to protected mode
	; First disable interrupts

	cli

	; Load a valid GDT (See the GDT description at the end)

	lgdt [GDT_Desc]
	
	; Enter protected mode by setting bit 0 of the CR0 register

	mov   eax, cr0                                       
	or    al , 1
	mov   cr0, eax

	; Switch CS to protected mode descriptor with a far jump	

	jmp   SEG32_CODE:ProtectedMode
	
	; Halt the system in case of trouble

    HaltSystem:
    mov   si, Messages.A20EnableErr
    .printchar:
    lodsb
    test  al, al
    jz    .printdone
    mov   ah, 0x0E
    mov   bx, 0x0007
    int   0x10
    jmp   .printchar

    .printdone:
    cli
    hlt
    jmp   .printdone


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Enable the A20 line

%include "x86/boot/src/a20.asm"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Messages:
.A20EnableErr db 'A20 line could not be enabled', 0

; Global Descriptor Table (GDT) : Tells the CPU about memory segments
; It is a table of 8-byte entries, each being a memory segment descriptor. The segment registers themselves point to the descriptors
; The first 8-bytes are expected to be NULL (0)
; Bytes 0-1 : 16 least significant bits of the 20-bit limit of the segment (either in units of 1 byte or 4 KB i.e. one page) ; 0xFFFF means the end of the 32-bit addressable space
; Bytes 2-4 : 24 least significant bits of the base address of the start of the segment
; Byte  5   : Access byte (description of bit-fields below)
; Byte  6   : First nibble contains the 4 most significant bits of the 20-bit limit of the segment ; most significant nibble contains page flags (description below)
; Byte  7   : 6 most significant bits of the base address of the start of the segment
;
; Access byte :
; - Bit 0   --> Set to 0 ; CPU sets this to 1 when the segment is accessed
; - Bit 1   --> Flag for segment readable (when code segment) and segment writable (when data segment) ; read always allowed for data segments ; write never allowed for code segments
; - Bit 2   --> Direction/conforming : For data segs 0 means segment grows up, 1 means segment grows down  
;                                      For code segs 0 means code can only be executed from ring set marked by "priv" bits ; 1 means code can be executed from same or lower privilege
; - Bit 3   --> Executable : If 1 code in this segment can be executed, i.e. a code selector; 0 for a data selector.
; - Bit 4   --> Descriptor type : Set for code or data segments ; cleared for system segments (eg. a Task State Segment)
; - Bit 5-6 --> Privilege (priv) bits : 0 highest privilege (kernel) and 3 lowest (user applications)
; - Bit 7   --> Present : Must be 1 for all valid selectors   
;
; Flag nibble : 
; - Bit 0   --> Available for software use (set to 0 by default)
; - Bit 1   --> Long mode (64-bit) segment if set 
; - Bit 2   --> Size : 0 for 16-bit protected mode of 80286 ; 1 for 32-bit protected mode
; - Bit 3   --> Granularity : If 0 limit value is in units of 1 byte ; if 1 limit value is in units of 1 page or 4 KB  
;
; We set up a GDT with 4 entries :
; - Null descriptor
; - 32-bit code segment descriptor
; - 32-bit data segment descriptor
; - 16-bit code segment descriptor
; - 16-bit data segment descriptor
; The segment descriptors implement the 'flat' memory model where the entire 32-bit address space is available to each segment

GDT: 
	dq 0
	dq 0x00CF9A000000FFFF
	dq 0x00CF92000000FFFF
	dq 0x000F9A000000FFFF
	dq 0x000F92000000FFFF

; Format of the GDT descriptor (6 bytes)
; Bytes 0-1 : Size of the GDT - 1
; Bytes 2-5 : Linear address of the table itself (paging applies)

GDT_Desc:
	dw GDT_Desc - GDT - 1
	dd GDT

times 512-($-$$) db 0
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; We are now in 32-bit protected mode

BITS 32

ProtectedMode:

