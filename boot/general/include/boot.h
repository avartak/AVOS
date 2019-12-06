#ifndef BOOT_GENERAL_BOOT_H
#define BOOT_GENERAL_BOOT_H

#include <boot/general/include/common.h>

#define BOOT_BLOCKLIST_MAXBLOCKS128 9
#define BOOT_BLOCKLIST_MAXBLOCKS512 41
#define BOOT_BLOCKLIST_MAXBLOCKS272 21

#define BOOT_32BIT_MEMORY_LIMIT     0xFFFFFFFF
#define BOOT_HIGH_MEMORY_START      0x100000

struct Boot_Block32 {
    uintptr_t address;
    size_t    size;
}__attribute__((packed));

struct Boot_Block64 {
    uint64_t address;
    uint64_t    size;
}__attribute__((packed));

struct Boot_BlockLBA {
	uint64_t  lba;
	size_t    num_sectors;
}__attribute__((packed));

struct Boot_BlockList128 {
    uint32_t  jump;
    uint32_t  load_address_lo;
    uint32_t  load_address_hi;
	uint16_t  sector_size;
    char      reserved[6];
    struct Boot_BlockLBA blocks[9];
}__attribute__((packed));

struct Boot_BlockList272 {
    uint32_t  jump;
    uint32_t  load_address_lo;
    uint32_t  load_address_hi;
	uint16_t  sector_size;
    char      reserved[6];
    struct Boot_BlockLBA blocks[21];
	char      string[240];
}__attribute__((packed));

struct Boot_BlockList512 {
	uint32_t  jump;
	uint32_t  load_address_lo;
	uint32_t  load_address_hi;
	uint16_t  sector_size;
    char      reserved[6];
	struct Boot_BlockLBA blocks[41];
}__attribute__((packed));

struct Boot_Kernel_Info {
    uint32_t  boot_drive_ID;
    uintptr_t pnpbios_check_ptr;
    uintptr_t boot_partition;
    uintptr_t part_info_ptr;
    uintptr_t blocklist_ptr;
    uintptr_t start;
    size_t    size;
    size_t    bss_size;
    uintptr_t multiboot_header;
    uintptr_t entry;
    uintptr_t file_addr;
    size_t    file_size;
}__attribute__((packed));

extern uint32_t Boot_MemoryBlockAboveAddress(uint32_t addr, uint32_t size, uint32_t align, struct Boot_Block64* mmap, size_t mmap_size);
extern uint32_t Boot_MemoryBlockBelowAddress(uint32_t addr, uint32_t size, uint32_t align, struct Boot_Block64* mmap, size_t mmap_size);

#endif
