BOOTSECTOR_MAGIC        equ 0xAA55                                      ; The magic number needed at the end of the boot sector

START_BOOT1             equ 0x7C00                                      ; This is where BIOS will load the boot sector
START_BOOT2             equ 0x8000                                      ; This is where we load the 2nd stage of the boot loader
START_KERNEL            equ 0x100000                                    ; Kernel is loaded at physical memory location of 1 MB
START_GDT               equ 0x7E00                                      ; Starting point of the area reserved for the protected mode descriptors (0xF000)
START_SCRATCH           equ 0x7000                                      ; Starting point of the scratch area 

SIZE_BOOT1              equ 0x0200                                      ; Boot sector is 512 bytes long
SIZE_BOOT2              equ 0x8000                                      ; We allow 32 KB for the 2nd stage of the boot loader - it is unlikely we will ever need this much space
SIZE_KERNEL             equ 0x100000                                    ; Size of the kernel is assumed to be 1 MB
SIZE_GDT                equ 0x0200                                      ; 512 bytes allotted to the GDT
SIZE_SCRATCH            equ 0x0400                                      ; 1 KB scratch area

STACK_TOP               equ 0x7000                                      ; Top of the stack - it can extend down till 0x500 without running into the BDA (0x400-0x500) and then the IVT (0-0x400)
SEG32_CODE              equ 0x08                                        ; 32-bit code segment
SEG32_DATA              equ 0x10                                        ; 32-bit data segment

FLOPPY_ID               equ 0                                           ; Floppy ID used by the BIOS
HDD_ID                  equ 0x80                                        ; Floppy ID used by the BIOS
SECTOR_SIZE             equ 0x200                                       ; Size of a sector on the disk : 512 bytes

START_BOOT1_DISK        equ 0x00                                        ; Boot sector (1st sector)
START_BOOT2_DISK        equ 0x01                                        ; Starting sector of the 2nd stage of the boot loader
START_KERNEL_DISK       equ 0x40                                        ; Starting sector of the kernel

SECTORS_PER_ITER        equ 0x80                                        ; We will copy the kernel 0x80 or 128 sectors at a time
KERNEL_COPY_ITER        equ SIZE_KERNEL/(SECTORS_PER_ITER*SECTOR_SIZE)  ; We need 16 iterations to load the 1 MB kernel

MULTIBOOT_MAGIC         equ 0x36d76289                                  ; Need to provide this as input to the kernel to convey that it has been loaded by a multiboot-2 compliant boot loader (which this is not)
MULTIBOOT_HEADER_SIZE   equ 0x1000                                      ; Multiboot2 header size, more specifically amount of bytes set aside for the header (it's actual size may be less)
MULTIBOOT_INFO_ADDRESS  equ 0x10000                                     ; Physical memory location of the multiboot information structures (MBI)
MULTIBOOT_INFO_SEGMENT  equ 0x0800
MULTIBOOT_INFO_OFFSET   equ 0x8000                                      ; Location of the start of the boot information structures 

