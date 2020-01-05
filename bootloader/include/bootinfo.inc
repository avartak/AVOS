MBR_SIZE               equ 0x0200                          ; Size of the MBR
VBR_SIZE               equ 0x0200                          ; Size of the VBR
MBR_RELOC_ADDRESS      equ 0x0600                          ; This is where the MBR relocates itself to before loading the VBR 
VBR_ADDRESS            equ 0x7C00                          ; This is where the MBR will load the VBR in memory

MBR_PART_TABLE_OFFSET  equ 0x01BE                          ; Offset of the start of the partition table in the MBR : byte 446

STACK_TOP              equ 0x7C00                          ; Top of the stack

BOOTLOADER_ADDRESS     equ 0x9000                          ; Starting location in memory where the bootloader code gets loaded

PARTITION_START_LBA    equ 0x0800                          ; LBA of the start sector of the boot partition
SECTOR_SIZE            equ 0x0200                          ; Size of a disk sector
