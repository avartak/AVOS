; We are now firmly in 32-bit protected mode

BITS 32

; First let us include some definitions of constants (the constants themselves are described in comments)

NUM_PDTPT_ENTRIES   equ 0x400                    ; There are 0x400 or 1024 entries in a page directory table or a page table
SIZE_PAGE           equ 0x1000                   ; Every page has a size of 4 KB
HIGHER_HALF_OFFSET  equ 0xC0000000               ; This is where we will map the kernel in high memory
KERN_TABLE_FLAGS    equ 0x3                      ; Attributes of the kernel page directory
KERN_PAGE_FLAGS     equ 0x3                      ; Attributes of the kernel page


; Lets include the multiboot header here -- our kernel is multiboot compliant

section .multiboot

%include "kernel/src/multiboot.asm"


; This is the start of out kernel -- we set up paging and launch into the higher half memory

section .text

extern Paging_kernel_directory
extern Paging_kernel_selftable

Directory  equ Paging_kernel_directory - HIGHER_HALF_OFFSET
Table      equ Paging_kernel_selftable - HIGHER_HALF_OFFSET

E820_Table equ 0x10000

global Kstart
Kstart:


CheckMemoryForKernelInit:

	mov ecx, DWORD [E820_Table]
	mov esi, E820_Table-0x14
	cmp ecx, 0
	je .mapcheckfail
	.checkfullmapping:
	add esi, 0x18
	mov eax, DWORD [esi+0x14]
	test eax, 1
	jz  .loop_on_checkfullmapping
	mov eax, DWORD [esi+0x10]
	cmp eax, 1
	jne .loop_on_checkfullmapping
	
	mov eax, DWORD [esi]
	mov ebx, DWORD [esi+8]
	add ebx, eax
	
	cmp eax, DWORD [Kernel_Start - HIGHER_HALF_OFFSET]
	jg .loop_on_checkfullmapping
	
	.checkendforfullmapping:
	cmp ebx, DWORD [Kernel_End - HIGHER_HALF_OFFSET]
	jge .mapchecksuccess

	.loop_on_checkfullmapping:
	loop .checkfullmapping



	mov ecx, DWORD [E820_Table]
	mov esi, E820_Table-0x14
	.checkpartialmapping:
	add esi, 0x18
    mov eax, [esi+0x14]
    test eax, 1
    jz  .loop_on_checkpartialmapping
    mov eax, [esi+0x10]
    cmp eax, 1
    jne .loop_on_checkpartialmapping
	mov eax, DWORD [esi]
	mov ebx, DWORD [esi+8]
	add ebx, eax

	mov dl, 0
	cmp eax, DWORD [Kernel_Start - HIGHER_HALF_OFFSET]
	jg .checkendforpartialmapping
	or  dl, 1

	.checkendforpartialmapping:
	cmp ebx, DWORD [Kernel_End - HIGHER_HALF_OFFSET]
	jl .checkbounds
	or  dl, 2

	.checkbounds:
	cmp dl, 0
	je  .loop_on_checkpartialmapping
	jmp .updatebounds	
	
	.loop_on_checkpartialmapping:
	loop .checkpartialmapping
	jmp .mapcheckfail

	.updatebounds:
	cmp dl, 3
	je .mapchecksuccess

	test dl, 1
	jnz .checkhighbound
	mov [Kernel_Start - HIGHER_HALF_OFFSET], eax

	.checkhighbound:
	test dl, 2
	jnz .do_checkpartialmapping_again
	mov [Kernel_End - HIGHER_HALF_OFFSET], ebx

	.do_checkpartialmapping_again:
	mov ecx, DWORD [E820_Table]
	mov esi, E820_Table-0x14
	loop .checkpartialmapping

	.mapchecksuccess:
	mov eax, 1
	jmp .endmapcheck

	.mapcheckfail:
	mov eax, 0

	.endmapcheck:
	test eax, 1
	jnz SetupPaging
	cli
	hlt


SetupPaging:
	mov ecx, NUM_PDTPT_ENTRIES-1                 ; There are 1024 or 0x400 entries in the page directory
	.fillPageDirectory:
		mov [Directory+4*ecx], DWORD 0           ; Initialize all entries to the page directory to 0. Basically none of the memory pages are accessible with this
		loop .fillPageDirectory                  ; Loop

    mov ecx, NUM_PDTPT_ENTRIES-1                 ; We will be creating one page table mapping physical memory from 0 to 4 MB (kernel space). There are 1024 or 0x400 entries in each page table
    .fillPageTables:
		mov eax, ecx
		mov ebx, SIZE_PAGE                       ; We will set entry i in the page table to (i * 0x1000) | 3 ; every entry in the page table corresponds to a 4 KB page table
		mul ebx                                  ; i * 0x1000 --> ith page and each page has a size of 4 KB                                  
		or  eax, 3                               ; 3 --> supervisor-only read/write access and page is present
		mov [Table+4*ecx], eax                   ; Note that each entry is 4 bytes long
		loop .fillPageTables

	mov eax, Table
	or  eax, 3                                   ; Here again we want to set the page table to have supervisor-only read/write access and be present
	mov [Directory]        , eax                 ; Identity map the first 4 MB -- w/o this the code will crash after after paging is enables
	mov [Directory+0x300*4], eax                 ; Map the kernel to 3 GB memory
	mov eax, Directory                           ; Recursively page the page directory as the last entry to itself
	or  eax, 3                                   ; Here again we want to set the page table to have supervisor-only read/write access and be present
	mov [Directory+0x3FF*4], eax                 ; Map the kernel to 3 GB memory

	mov eax, Directory                           ; Put the address of the page directory in CR3
	mov cr3, eax       

	mov eax, cr0                                 ; Enable paging in CR0 by turning on the 32nd bit 
	or  eax, 0x80000000
	mov cr0, eax

	jmp 0x8:HigherHalf

HigherHalf:

	mov eax, 0                                   ; Remove the identity mapping -- we no longer need it 
	mov [Directory], eax

	mov esp, 0xC0400000                          ; Set the stack pointer to 4 MB physical memory address

    extern Kmain
    jmp Kmain



Kernel_Start_Memory:

Kernel_Start dd 0x100000
Kernel_End   dd 0x800000

