section .multiboot

BITS 32

extern __header_addr
extern __load_addr
extern __load_end_addr
extern __bss_end_addr
extern __entry_addr

Multiboot2_Header_start:

global Multiboot2_Header_magic
Multiboot2_Header_magic:
	.magic          dd    0xE85250D6
	.architecture   dd    0
	.header_length  dd    Multiboot2_Header_end - Multiboot2_Header_start
	.checksum       dd    0x100000000 - (0xE85250D6 + 0 + (Multiboot2_Header_end - Multiboot2_Header_start))

global Multiboot2_Header_tag1
Multiboot2_Header_tag1_start:
Multiboot2_Header_tag1:
	.type           dw    1
	.flags          dw    1
	.size           dd    Multiboot2_Header_tag1_end - Multiboot2_Header_tag1_start
	.mbi_tag_type1  dd    1
	.mbi_tag_type2  dd    2
	.mbi_tag_type4  dd    4
	.mbi_tag_type6  dd    6
	.mbi_tag_type7  dd    7
	.mbi_tag_type8  dd    8
	.mbi_tag_type10 dd   10
	.mbi_tag_type13 dd   13
	.mbi_tag_type14 dd   14
	.mbi_tag_type15 dd   15
	.mbi_tag_type21 dd   21
	.mbi_tag_type99 dd   99
Multiboot2_Header_tag1_end:

global Multiboot2_Header_tag2
Multiboot2_Header_tag2_start:
Multiboot2_Header_tag2:
    .type           dw    2
    .flags          dw    0
    .size           dd    Multiboot2_Header_tag2_end - Multiboot2_Header_tag2_start
    .header_addr    dd    __header_addr
    .load_addr      dd    __load_addr
    .load_end_addr  dd    __load_end_addr
    .bss_end_addr   dd    __bss_end_addr
Multiboot2_Header_tag2_end:

global Multiboot2_Header_tag3
Multiboot2_Header_tag3_start:
Multiboot2_Header_tag3:
    .type           dw    3
    .flags          dw    0
    .size           dd    Multiboot2_Header_tag3_end - Multiboot2_Header_tag3_start
    .entry_addr     dd    __entry_addr
Multiboot2_Header_tag3_end:

global Multiboot2_Header_tag4
Multiboot2_Header_tag4_start:
Multiboot2_Header_tag4:
    .type           dw    4
    .flags          dw    0
    .size           dd    Multiboot2_Header_tag4_end - Multiboot2_Header_tag4_start
    .console_flags  dd    3
Multiboot2_Header_tag4_end:

global Multiboot2_Header_tag5
Multiboot2_Header_tag5_start:
Multiboot2_Header_tag5:
    .type           dw    5
    .flags          dw    0
    .size           dd    Multiboot2_Header_tag5_end - Multiboot2_Header_tag5_start
    .width          dd    80
    .height         dd    25
    .depth          dd    0
Multiboot2_Header_tag5_end:

global Multiboot2_Header_term
Multiboot2_Header_term_start:
Multiboot2_Header_term:
    .type           dw    0
    .flags          dw    0
    .size           dd    8
Multiboot2_Header_term_end:

Multiboot2_Header_end:



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


KERNEL_STACK_PTR_OFFSET   equ 0x6000
KERNEL_HIGHER_HALF_OFFSET equ 0xC0000000

extern Paging_Initialize
extern Paging_SwitchToHigherHalf
extern CRT_Initialize
extern GDT_Initialize
extern IDT_Initialize
extern Welcome
extern AVOS

section .text

BITS 32

global Start
Start:

	cli
	cld 

	mov   esp, Start+KERNEL_STACK_PTR_OFFSET-KERNEL_HIGHER_HALF_OFFSET

	cmp   eax, 0x36d76289
	jne   HaltSystem

	add   ebx, KERNEL_HIGHER_HALF_OFFSET
	mov   [BootInfo_Ptr-KERNEL_HIGHER_HALF_OFFSET], ebx


	call  Paging_Initialize
	call  Paging_SwitchToHigherHalf
	call  CRT_Initialize
	call  GDT_Initialize
	call  IDT_Initialize
	call  Welcome

	jmp   AVOS

	HaltSystem:
    cli
    hlt
    jmp  HaltSystem

	times KERNEL_STACK_PTR_OFFSET-($-$$) db 0

section .data

	global BootInfo_Ptr
	BootInfo_Ptr:
	dd 0
