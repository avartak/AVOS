PARTITION_TABLE_OFFSET equ 0x01BE                          ; Offset of the start of the partition table in the MBR (or some VBRs) : byte 446
PARTITION_START_LBA    equ 0x0800                          ; LBA of the start sector of the boot partition
PARTITION_ATTRIBUTES   equ 0x80                            ; This defines the partition to be active/bootable [could be modified by the partition manager]
PARTITION_TYPE         equ 0                               ; Partition identifier [could be modified by the partition manager]
PARTITION_SIZE         equ 0xFFFFFFFF                      ; Size (number) of sectors of the boot partition [could be modified by the partition manager]

SECTOR_SIZE            equ 0x0200                          ; Size of a disk sector

VBR_ADDRESS            equ 0x7C00                          ; This is where the MBR will load the VBR in memory
BOOTLOADER_ADDRESS     equ 0x7E00                          ; Starting location in memory where the bootloader code gets loaded

BOOT_SIGNATURE_OFFSET  equ 510                             ; Offset within an MBR or VBR where the boot signature is put
