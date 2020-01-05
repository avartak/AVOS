
VBR_ADDRESS            equ 0x7C00                          ; This is where the MBR will load the VBR in memory
BOOTLOADER_ADDRESS     equ 0x9000                          ; Starting location in memory where the bootloader code gets loaded

PARTITION_START_LBA    equ 0x0800                          ; LBA of the start sector of the boot partition
SECTOR_SIZE            equ 0x0200                          ; Size of a disk sector
