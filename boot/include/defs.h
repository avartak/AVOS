#ifndef BOOT_DEFS_H
#define BOOT_DEFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MEMORY_NULL_PTR           ((void*)0xFFFFFFFF)
#define MEMORY_32BIT_LIMIT        0xFFFFFFFF
#define MEMORY_HIGH_START         0x100000

#define BLOCKLIST_MAXBLOCKS128    9
#define BLOCKLIST_MAXBLOCKS512   41
#define BLOCKLIST_MAXBLOCKS272   21

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

struct Boot_KernelInfo {
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

#endif
