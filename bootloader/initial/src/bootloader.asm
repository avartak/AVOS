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

%include "bootloader/initial/include/bootinfo.inc"

; Starting point of the bootloader in memory --> follows immediately after the 512 bytes of the VBR

section .text

; We are still in 16-bit real mode

BITS 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

global BOOT
BOOT:

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
	
	.Address              dq 0
	.Sector_Size          dw SECTOR_SIZE
	.Reserved1            dw 0
	.Reserved2            dd 0
	
	.Block1_LBA           dq 0x1800+PARTITION_START_LBA 
	.Block1_Num_Sectors   dd 1
	
	; Pad the remaining bytes up to BOOT+128 with zero -- 128 = 124 (blocklist) + 4 (JMP 2 bytes + 2 NOPs)
	
	times 128-($-$$)      db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	; This is where the code starts

	Code:

	; Expect the boot drive ID in DL and the active partition table entry in the relocated MBR in DS:SI when control is transferred to the boot loader. 
	; ES:DI may point to "$PnP" installation check structure for systems with Plug-and-Play BIOS or BBS support
	; To pass on to the protected mode, save DS:SI in ESI, ES:DI in EDI, blocklist pointer to the kernel and kernel modules in EBX

	movzx esi, si
	xor   eax, eax
	mov   ax, ds
	shl   eax, 4
	add   esi, eax

	movzx edi, di
	xor   eax, eax
	mov   ax, es
	shl   eax, 4
	add   esi, eax

	xor   ebx, ebx
	mov   ebx, BOOT

	; Set all the segment registers to the base address we want (0x0000), and set up the stack pointer
	
	xor   ax, ax
	mov   ds, ax	
	mov   es, ax	
	mov   ss, ax
	mov   sp, STACK_TOP
	jmp   0x0000:EnableA20

	; Code to enable the A20 line

	extern A20_Enable

	EnableA20:
	call  A20_Enable
	test  al, al
	jnz   EnablePMode

	; Halt the system with an error message if A20 enable fails

	mov   si, A20EnableErr

	.printerr:
	lodsb
	test  al, al
	jz    .hltloop
	mov   ah, 0x0E
	mov   bx, 0x0007
	int   0x10
	jmp   .printerr
	
	.hltloop:
	cli
	hlt
	jmp   .hltloop



	; Switch to protected mode

	EnablePMode:

	; First disable interrupts

	cli

	; Load a valid GDT (See the GDT description at the end)

	lgdt  [GDT.Pointer]
	
	; Enter protected mode by setting bit 0 of the CR0 register, and making a long-jump using a 32-bit code segment (which switches CS to protected mode)

	mov   eax, cr0                                       
	or    al , 1
	mov   cr0, eax

	jmp   GDT.Code32:ProtectedMode

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; We are now in 32-bit protected mode

section .text

BITS 32

	; Switch all registers to protected mode
	
	ProtectedMode:
	mov   ax, GDT.Data32
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax
	mov   esp, STACK_TOP
	
	; We need to store the information passed on by the VBR into a kernel information structure to be used by the bootloader code (written in C)
	
	mov   [Kernel_Info.boot_drive_ID],   dl
	mov   [Kernel_Info.boot_partition], esi
	mov   [Kernel_Info.pnpbios_ptr],    edi
	mov   [Kernel_Info.blocklist_ptr],  ebx
	
	; Boot OS 
	
	extern Multiboot_Boot
	
	push  Kernel_Info
	push  Multiboot_MBI
	call  Multiboot_Boot
	add   esp, 0x8
	test  al, al
	jz    ProtectedMode.hltloop
	
	; Store the pointer to the boot information table in EBX
	
	mov   ebx, Multiboot_MBI
	
	; Store the Multiboot2 bootloader magic value in EAX
	
	mov   eax, 0x36d76289
	
	; Jump to the kernel
	
	jmp   [Kernel_Info.entry]
	
	.hltloop:
	cli
	hlt
	jmp   .hltloop

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

section .data

; Error messages 

A20EnableErr   db 'A20 line not enabled', 0

; Description of the GDT is given at the end 

align 8

GDT:
	.Null   : equ $-GDT 
	dq 0
	.Code32 : equ $-GDT 
	dq 0x00CF9A000000FFFF
	.Data32 : equ $-GDT 
	dq 0x00CF92000000FFFF
	.Code16 : equ $-GDT 
	dq 0x000F9A000000FFFF
	.Data16 : equ $-GDT 
	dq 0x000F92000000FFFF
	.Pointer:
	dw $-GDT-1
	dd GDT

; Kernel loading information

global Kernel_Info
Kernel_Info:
	.boot_drive_ID     dd 0
	.boot_partition    dd 0
	.pnpbios_ptr       dd 0
	.blocklist_ptr     dd 0
	.start             dd 0
	.size              dd 0
	.bss_size          dd 0
	.multiboot_header  dd 0
	.entry             dd 0
	.file_addr         dd 0
	.file_size         dd 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


section .bss

align 8

; Multiboot information (MBI) table

global Multiboot_MBI
Multiboot_MBI:


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

