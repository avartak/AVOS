BOOTSECT_MAGIC    equ 0xAA55                                      ; The magic number needed at the end of the boot sector

SIZE_BOOT1        equ 0x0200                                      ; Boot sector is 512 bytes long
SIZE_BOOT2        equ 0x2000                                      ; Stage of the boot loader (16-bit) is assumed to be 8 KB
SIZE_BOOT3        equ 0x2000                                      ; Stage of the boot loader (32-bit) is assumed to be 8 KB
SIZE_STACK        equ 0x2000                                      ; Stack is assigned a size of 8 KB
SIZE_SCRCH        equ 0x1000                                      ; A 4 KB scratch area is put aside
SIZE_DESCR        equ 0x1000                                      ; 4 KB area allotted to the GDT and IDT
SIZE_KERNL        equ 0x100000                                    ; Size of the kernel is assumed to be 1 MB


START_BOOT1       equ 0x7C00                                      ; This is where BIOS will load the boot sector
END_BOOT1         equ START_BOOT1 + SIZE_BOOT1                    

START_BOOT2       equ 0x8000                                      ; This is where we load the 2nd stage of the boot loader
END_BOOT2         equ START_BOOT2 + SIZE_BOOT2

START_BOOT3       equ END_BOOT2                                   ; This is where we load the 3rd stage of the boot loader (0xA000)
END_BOOT3         equ START_BOOT3 + SIZE_BOOT3

START_STACK       equ END_BOOT3                                   ; Starting point of the stack segment (0xC000)
END_STACK         equ START_STACK + SIZE_STACK

START_SCRCH       equ END_STACK                                   ; Starting point of the scratch area (0xE000)
END_SCRCH         equ START_SCRCH + SIZE_SCRCH

START_DESCR       equ END_SCRCH                                   ; Starting point of the area reserved for the protected mode descriptors (0xF000)
END_DESCR         equ START_DESCR + SIZE_DESCR

START_KERNL       equ 0x100000                                    ; Kernel is loaded at physical memory location of 1 MB
END_KERNL         equ START_KERNL + SIZE_KERNL

SEG_DS16          equ 0x0                                         ; 16-bit data segment
SEG_ES16          equ 0x0                                         ; 16-bit extra segment
SEG_SS16          equ 0x0C00                                      ; 16-bit stack segment

SEG_CS32          equ 0x08                                        ; 32-bit code segment
SEG_DS32          equ 0x10                                        ; 32-bit data segment
SEG_ES32          equ 0x10                                        ; 32-bit extra segment
SEG_FS32          equ 0x10                                        ; 32-bit FS segment
SEG_GS32          equ 0x10                                        ; 32-bit GS segment
SEG_SS32          equ 0x10                                        ; 32-bit stack segment

FLOPPY_ID         equ 0                                           ; Floppy ID used by the BIOS
SECTOR_SIZE       equ 0x200                                       ; Size of a sector on the disk : 512 bytes

SIZE_BOOT1_DISK   equ SIZE_BOOT1/SECTOR_SIZE
SIZE_BOOT2_DISK   equ SIZE_BOOT2/SECTOR_SIZE
SIZE_BOOT3_DISK   equ SIZE_BOOT3/SECTOR_SIZE
SIZE_KERNL_DISK   equ SIZE_KERNL/SECTOR_SIZE

START_BOOT1_DISK  equ 0x00                                        ; Boot sector (1st sector)
START_BOOT2_DISK  equ 0x08                                        ; Starting sector of the 2nd stage of the boot loader
START_BOOT3_DISK  equ START_BOOT2_DISK + SIZE_BOOT2_DISK          ; Starting sector of the 3rd stage of the boot loader
START_KERNL_DISK  equ START_BOOT3_DISK + SIZE_BOOT3_DISK          ; Starting sector of the kernel

SECTRS_PER_ITER   equ 0x80                                        ; We will copy the kernel 0x80 or 128 sectors at a time
KERNL_COPY_ITER   equ SIZE_KERNL/(SECTRS_PER_ITER*SECTOR_SIZE)    ; We need 16 iterations to load the 1 MB kernel

NUM_PDTPT_ENTRIES equ 0x400                                       ; There are 0x400 or 1024 entries in a page directory table or a page table
START_PDT         equ 0x10000                                     ; Starting point of the page directory table
SIZE_PAGE         equ 0x1000                                      ; Every page has a size of 4 KB

KERNL_HH          equ 0xC0100000                                  ; This is where we will map the kernel in high memory
LOC_MBOOT_SIZE    equ 0x8                                         ; Location in the multiboot header of the 32-bit double word containing the header size 
