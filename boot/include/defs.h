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
    uint32_t  address;
    uint32_t  size;
}__attribute__((packed));

struct Boot_Block64 {
    uint64_t  address;
    uint64_t  size;
}__attribute__((packed));

struct Boot_BlockLBA {
	uint64_t  lba;
	uint32_t  num_sectors;
}__attribute__((packed));

struct Boot_BlockList128 {
    uint32_t  jump;
    uint64_t  address;
	uint16_t  sector_size;
    char      reserved[6];
    struct Boot_BlockLBA blocks[9];
}__attribute__((packed));

struct Boot_BlockList272 {
    uint32_t  jump;
    uint64_t  address;
	uint16_t  sector_size;
    char      reserved[6];
    struct Boot_BlockLBA blocks[21];
	char      string[240];
}__attribute__((packed));

struct Boot_BlockList512 {
	uint32_t  jump;
	uint64_t  address;
	uint16_t  sector_size;
    char      reserved[6];
	struct Boot_BlockLBA blocks[41];
}__attribute__((packed));

struct Boot_KernelInfo {
    uint32_t  boot_drive_ID;
    uint32_t  boot_partition;
    uint32_t  pnpbios_ptr;
    uint32_t  blocklist_ptr;
    uint32_t  start;
    uint32_t  size;
    uint32_t  bss_size;
    uint32_t  multiboot_header;
    uint32_t  entry;
    uint32_t  file_addr;
    uint32_t  file_size;
}__attribute__((packed));

#endif
