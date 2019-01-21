; This is the code of the 3rd stage of the boot loader
; We are in protected mode
; This code is expected to reside at memory starting from 0xA000
; The goal is to enable paging, and launch the higher half kernel
; The kernel (16 KB for now) has already been copied to the physical address 1 MB
; The first 4 MB will be identity mapped, then mapped to 3 GB
; The kernel code will be linked to resolve labels at 3 GB, so the linker is assuming that the (virtual) memory is already putting the kernel at a high location 

; First let us include some definitions of constants (the constants themselves are described in comments)

%include "x86/boot/src/defs.asm"

; Starting point of the kernel loader

ORG START_BOOT3

; We are now firmly in 32-bit protected mode

BITS 32

; So lets discuss the memory map of the kernel
; We will leave the first MB of physical memory untouched 
; The kernel will be loaded to 1 MB physical memory and 3 GB virtual memory
; The assumed size of the kernel is 1 MB (that's how much we have read from the floppy) -- this includes the code and initialized data
; The heap will be assigned 1 MB starting at physical address 2 MB
; The stack will be 1 MB ranging between 3 MB and 4 MB of physical memory
; The page tables require at most 4 MB and will be assigned physical memory space from 4 MB to 8 MB
; This should all be done by the kernel itself

; The 3rd stage kernel loader will for now create the page directory and the page table for the kernel at the following locations
; Memory locations of the page directory and the page table for the first 4 MB of physical memory

Page_Directory equ START_PDT
Kernel_Table   equ START_KPT

Kload32: 

	cli                                          ; Lets clear the interrupts. They are already cleared but lets do it anyways

	mov ecx, NUM_PDTPT_ENTRIES-1                 ; There are 1024 or 0x400 entries in the page directory
	.fillPageDirectory:
		mov [Page_Directory+4*ecx], DWORD 0      ; Initialize all entries to the page directory to 0. Basically none of the memory pages are accessible with this
		loop .fillPageDirectory                  ; Loop

    mov ecx, NUM_PDTPT_ENTRIES-1                 ; There are 1024 or 0x400 entries in the page table
    .fillKernelTable:
        mov eax, ecx
        mov ebx, SIZE_PAGE                       ; We will set entry i in the page table to (i * 0x1000) | 3 to set up the table for the first 4 MB of physical memory
        mul ebx                                  ; i * 0x1000 --> ith page and each page has a size of 4 KB                                  
        or eax, 3                                ; 3 --> supervisor-only read/write access and page is present
        mov [Kernel_Table+4*ecx], eax            ; Note that each entry is 4 bytes long
		loop .fillKernelTable

	mov eax, Kernel_Table
	or eax, 3                                    ; Here again we want to set the page table to have supervisor-only read/write access and be present
	mov [Page_Directory]        , eax            ; Identity map the first 4 MB -- w/o this the code will crash after after paging is enables
	mov [Page_Directory+0x300*4], eax            ; Map the kernel to 3 GB memory
	                                             ; We have both an identity map and a higher half map of the kernel. The kernel should get rid of the identity map but keep the higher half mapping

	mov eax, Page_Directory                      ; Put the address of the page directory in CR3
	mov cr3, eax       

	mov eax, cr0                                 ; Enable paging in CR0 by turning on the 32nd bit 
	or eax, 0x80000000
	mov cr0, eax

	mov eax, KERNL_HH                            ; Jump to the start of the kernel -- we have the multiboot header at the start of the higher half location of the kernel
	add eax, [KERNL_HH+LOC_MBOOT_SIZE]           ; The size of the multiboot header is stored in a double word at location LOC_MBOOT_SIZE of the header
	jmp eax                                      
                                                 





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

