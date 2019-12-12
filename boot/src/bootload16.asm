; This is the code of the 16-bit (REAL mode) part of the bootloader
; We are still in REAL mode
; This code makes the switch to PROTECTED mode and transfers control to the 32-bit boot loader (largely written in C) that will then copy the kernel to memory
; To enable the protected mode we will :
; - Switch on the A20 line 
; - Load a valid GDT
; - Set the first bit in CR0 to 1

; Some notes about the mapping of the first MB of the physical memory in the PC architecture, and how we use this memory 

; 0x000000 - 0x000400 : Interrupt Vector Table  
; 0x000400 - 0x000500 : BIOS data area          
; 0x000500 - 0x007C00 : Usable
; 0x000600 - 0x000800 : Relocated MBR
; 0x000800 - 0x007C00 : Free
; 0x007C00 - 0x007E00 : VBR
; 0x007E00 - 0x008000 : 16-bit part of the boot loader code
; 0x008000 - 0x09FC00 : 32-bit part of the boot loader code + free space
; 0x09FC00 - 0x0A0000 : Extended BIOS data area 
; 0x0A0000 - 0x0C0000 : Video memory            
; 0x0C0000 - 0x100000 : BIOS            

; First let us include some definitions of constants

AVBL16_ADDRESS          equ 0x7E00                                      ; Starting location in memory where the 16-bit part of AVBL is loaded
AVBL32_ADDRESS          equ 0x8000                                      ; Starting location in memory where the 32-bit part of AVBL is loaded
SEG32_CODE              equ 0x08                                        ; 32-bit code segment
SEG32_DATA              equ 0x10                                        ; 32-bit data segment

PARTITION_START_LBA     equ 0x0800                                      ; LBA of the start sector of the boot partition [could be modified]

; Starting point of the bootloader in memory --> follows immediately after the 512 bytes of the VBR

ORG AVBL16_ADDRESS

; We are still in 16-bit real mode

BITS 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

AVBL:

	jmp   Code
	nop
	nop
	
	BlockList:
	
	; We reserve the next 124 bytes for the blocklist master for the blocklists of the kernel and modules to be loaded from disk to memory
	; This is basically a table containing (up to) 8 twelve-byte entries + 1 null entry (zero size)
	; The first 8 bytes of the blocklist contain the 64-bit load address that can be ignored in this specific case
	; Next 8-bytes are reserved
	; Then come the blocklist entries
	; First 8 bytes of each entry contain the 64-bit LBA of the start sector of a 'block' of the blocklists file
	; The last 4 bytes of each entry contain the size of the block (number of contiguous sectors to be read out)
	; An entry with 0 size marks the end of the blocklist, all remaining entries will be ignored
	
	.Load_Address         dq 0
	.Sector_Size          dw 0x200
	.Reserved1            dw 0
	.Reserved2            dd 0
	
	.Block1_LBA           dq 0x1800+PARTITION_START_LBA
	.Block1_Num_Sectors   dd 1
	
	; Pad the remaining bytes up to AVBL+128 with zero -- 128 = 124 (blocklist) + 4 (JMP 2 bytes + 2 NOPs)
	
	times 128-($-$$)      db 0

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

	; Set all the segment registers to the base address we want (0x0000)
	
	xor   ax, ax
	mov   ds, ax	
	mov   es, ax	
	jmp   0x0000:EnableA20

	; Code to enable the A20 line

	%include "boot/src/a20.asm"

	EnableA20:
	call  A20_Enable
	test  al, al
	jnz   SaveInfo

	; Halt the system with an error message if A20 enable fails

	mov   si, A20EnableErr

	HaltSystem:
	lodsb
	test  al, al
	jz    .hltloop
	mov   ah, 0x0E
	mov   bx, 0x0007
	int   0x10
	jmp   HaltSystem
	
	.hltloop:
	cli
	hlt
	jmp   .hltloop

	A20EnableErr   db 'A20 line not enabled', 0

	; Save the boot drive ID in DL, the location of MBR's active partition in ESI, and information about "$PnP" installation check structure in EDI
	; Save the pointer to the kernel code blocklist in EBX

	SaveInfo:

	xor   eax, eax

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

	mov   ebx, AVBL
	pop   dx

	; Switch to protected mode
	; First disable interrupts

	cli

	; Load a valid GDT (See the GDT description at the end)

	lgdt  [GDT_Desc]
	
	; Enter protected mode by setting bit 0 of the CR0 register, and making a long-jump using a 32-bit code segment (which switches CS to protected mode)

	mov   eax, cr0                                       
	or    al , 1
	mov   cr0, eax

	jmp   SEG32_CODE:ProtectedMode

	; Description of the GDT is given at the end 

	GDT:
    dq 0
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
    dq 0x000F9A000000FFFF
    dq 0x000F92000000FFFF

	GDT_Desc:
    dw GDT_Desc - GDT - 1
    dd GDT

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; We are now in 32-bit protected mode

BITS 32

	; Switch all registers to protected mode

	ProtectedMode:
	mov   ax, SEG32_DATA
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax

	jmp   AVBL32_ADDRESS

	times 0x200-($-$$) db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Global Descriptor Table (GDT) : Tells the CPU about memory segments
; It is a table of 8-byte entries, each being a memory segment descriptor. The segment registers themselves point to the descriptors
; The first 8-bytes are expected to be NULL (0)
;
;-----------------------------------------------------------------------------
;|31                                 16 | 15                               0 |
;|----------------------------------------------------------------------------
;|                                      |                                    |
;|                 Base                 |                Limit               |
;|                 0:15                 |                0:15                |
;|                                      |                                    |
;-----------------------------------------------------------------------------
;-----------------------------------------------------------------------------
;|63              56 |55   52 |51    48 | 47            40|39             32 |
;|----------------------------------------------------------------------------
;|                   |        |         |                 |                  |
;|       Base        | Flags  |  Limit  |   Access byte   |       Base       |
;|       24:31       |        |  16:19  |                 |       16:23      |
;|                   |        |         |                 |                  |
;-----------------------------------------------------------------------------
;
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
; In order to load the GDT we use the 'lgdt' instruction and pass to it the address of the GDT descriptor
; Format of the GDT descriptor (6 bytes)
; Bytes 0-1 : Size of the GDT - 1
; Bytes 2-5 : Linear address of the table itself (paging applies)
;
; We set up a GDT with 4 entries :
; - Null descriptor
; - 32-bit code segment descriptor
; - 32-bit data segment descriptor
; - 16-bit code segment descriptor
; - 16-bit data segment descriptor
; The segment descriptors implement the 'flat' memory model where the entire 32-bit address space is available to each segment

