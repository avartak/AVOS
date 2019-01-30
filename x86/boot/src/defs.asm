BOOTSECT_MAGIC    equ 0xAA55                                      ; The magic number needed at the end of the boot sector

SIZE_BOOT1        equ 0x0200                                      ; Boot sector is 512 bytes long
SIZE_BOOT2        equ 0x8000                                      ; We allow 32 KB for the 2nd stage of the boot loader - it is unlikely we will ever need this much space
SIZE_SCRCH        equ 0x0400                                      ; 1 KB scratch area
SIZE_GDT          equ 0x0200                                      ; 512 bytes allotted to the GDT
SIZE_KERNL        equ 0x100000                                    ; Size of the kernel is assumed to be 1 MB

START_SCRCH       equ 0x7000                                      ; Starting point of the scratch area 
END_SCRCH         equ START_SCRCH + SIZE_SCRCH                    ; It's size is 1 KB, so it ends at 0x7400 but we can with it spilling over till 0x7C00

TOP_STACK         equ 0x7000                                      ; Top of the stack - it can extend down till 0x500 without running into the BDA (0x400-0x500) and then the IVT (0-0x400)
                                                                  ; That's 26 KB, more than the stack should ever need

START_BOOT1       equ 0x7C00                                      ; This is where BIOS will load the boot sector
END_BOOT1         equ START_BOOT1 + SIZE_BOOT1                    

START_GDT         equ 0x7E00                                      ; Starting point of the area reserved for the protected mode descriptors (0xF000)
END_GDT           equ START_GDT + SIZE_GDT

START_BOOT2       equ 0x8000                                      ; This is where we load the 2nd stage of the boot loader
END_BOOT2         equ START_BOOT2 + SIZE_BOOT2

START_KERNL       equ 0x100000                                    ; Kernel is loaded at physical memory location of 1 MB
END_KERNL         equ START_KERNL + SIZE_KERNL

START_MEM_MAP     equ 0x8000                                      ; Location of the start of the memory map read out by INT 0x15, AH=0xE820	

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
HDD_ID            equ 0x80                                        ; Floppy ID used by the BIOS
SECTOR_SIZE       equ 0x200                                       ; Size of a sector on the disk : 512 bytes

SIZE_BOOT1_DISK   equ SIZE_BOOT1/SECTOR_SIZE
SIZE_BOOT2_DISK   equ SIZE_BOOT2/SECTOR_SIZE
SIZE_KERNL_DISK   equ SIZE_KERNL/SECTOR_SIZE

START_BOOT1_DISK  equ 0x00                                        ; Boot sector (1st sector)
START_BOOT2_DISK  equ 0x01                                        ; Starting sector of the 2nd stage of the boot loader
START_KERNL_DISK  equ 0x40                                        ; Starting sector of the kernel

SECTRS_PER_ITER   equ 0x80                                        ; We will copy the kernel 0x80 or 128 sectors at a time
KERNL_COPY_ITER   equ SIZE_KERNL/(SECTRS_PER_ITER*SECTOR_SIZE)    ; We need 16 iterations to load the 1 MB kernel

KERNEL_START      equ 0x100000                                    ; Physical location of the start of the kernel
MBOOT_SIZE_PTR    equ KERNEL_START+0x8                            ; Location where the size of the multiboot header is stored
