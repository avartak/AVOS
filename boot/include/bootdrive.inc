PARTITION_START_LBA    equ 0x0800                          ; LBA of the start sector of the boot partition
PARTITION_ATTRIBUTES   equ 0x80                            ; This defines the partition to be active/bootable [could be modified by the partition manager]
PARTITION_TYPE         equ 0                               ; Partition identifier [could be modified by the partition manager]
PARTITION_SIZE         equ 0xFFFFFFFF                      ; Size on sectors of the boot partition [could be modified by the partition manager]

SECTOR_SIZE            equ 0x0200                          ; Size of a disk sector

VBR_ADDRESS            equ 0x7C00                          ; This is where the MBR will load the VBR in memory
