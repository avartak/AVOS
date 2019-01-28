; This is the code of the 3rd stage of the boot loader
; This is what we expect from the system at this stage (or this is where we expect the 2nd stage of the boot loader to put is)
; - In protected mode with the PE bit (first bit) of CR0 set
; - All the segments registers (CS, DS, ES, FS, GS, SS) set to valid entries in a GDT ; Needless to say these segments should have kernel access rights
; - Maskable interrupts (IF bit 9 in EFLAGS) are cleared
; - The code is residing at memory starting from 0xA000
; - The stack is in an udefined state ; set the stack pointer if the stack is going to be used here
; - The kernel has been copied to physical memory 1 MB
;
; The goal is to enable paging, and map the kernel to higher half virtual memory
; We will also map physical memory 4 MB - 8 MB to high memory. This will be used for page maps for the entire physical memory
; The first 8 MB will be identity mapped, and also mapped to 3 GB
; At the end of it the kernel will have both identity access and higher half access
; The kernel code should not assume that it is mapped to higher half, but should redo the paging itself again
; This way, we could compile the kernel into a binary with origin set to 3 GB or we could compile it as an ELF binary and wun it with a multiboot compliant boot loader (this has not been tested)

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
; The 3rd stage kernel loader will for now create the page directory and the page table for the kernel at the following locations

Page_Directory equ START_PDT
Page_Tables    equ START_PDT + NUM_PDTPT_ENTRIES*4

Kload32: 

	mov ecx, NUM_PDTPT_ENTRIES-1                 ; There are 1024 or 0x400 entries in the page directory
	.fillPageDirectory:
		mov [Page_Directory+4*ecx], DWORD 0      ; Initialize all entries to the page directory to 0. Basically none of the memory pages are accessible with this
		loop .fillPageDirectory                  ; Loop

    mov ecx, NUM_PDTPT_ENTRIES-1                 ; We will be creating one page table mapping physical memory from 0 to 4 MB (kernel space). There are 1024 or 0x400 entries in each page table
    .fillPageTables:
		mov eax, ecx
		mov ebx, SIZE_PAGE                       ; We will set entry i in the page table to (i * 0x1000) | 3 ; every entry in the page table corresponds to a 4 KB page table
		mul ebx                                  ; i * 0x1000 --> ith page and each page has a size of 4 KB                                  
		or  eax, 3                               ; 3 --> supervisor-only read/write access and page is present
		mov [Page_Tables+4*ecx], eax             ; Note that each entry is 4 bytes long
		loop .fillPageTables

	mov eax, Page_Tables
	or  eax, 3                                   ; Here again we want to set the page table to have supervisor-only read/write access and be present
	mov [Page_Directory]        , eax            ; Identity map the first 4 MB -- w/o this the code will crash after after paging is enables
	mov [Page_Directory+0x300*4], eax            ; Map the kernel to 3 GB memory
	mov eax, Page_Directory                      ; Recursively page the page directory as the last entry to itself
	or  eax, 3                                   ; Here again we want to set the page table to have supervisor-only read/write access and be present
	mov [Page_Directory+0x3FF*4], eax            ; Map the kernel to 3 GB memory

	mov eax, Page_Directory                      ; Put the address of the page directory in CR3
	mov cr3, eax       

	mov eax, cr0                                 ; Enable paging in CR0 by turning on the 32nd bit 
	or  eax, 0x80000000
	mov cr0, eax

	mov eax, 0                                   ; Remove the identity mapping -- we no longer need it 
	mov [Page_Directory], eax

	mov esp, 0xC0400000                          ; Set the stack pointer to 4 MB physical memory address

	jmp KERNL_HH                                      
                                                 





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

