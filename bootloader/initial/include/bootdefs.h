#ifndef BOOTLOADER_BOOTDEFS_H
#define BOOTLOADER_BOOTDEFS_H

#include <stdint.h>
#include <stdbool.h>

#define BLOCKLIST_MAXBLOCKS128    9
#define BLOCKLIST_MAXBLOCKS512   41
#define BLOCKLIST_MAXBLOCKS272   21

struct BootInfo_BlockRAM {
    uint64_t  address;
    uint64_t  size;
}__attribute__((packed));

struct BootInfo_BlockLBA {
	uint64_t  lba;
	uint32_t  num_sectors;
}__attribute__((packed));

struct BootInfo_BlockList128 {
    uint32_t  jump;
    uint64_t  address;
	uint16_t  sector_size;
    char      reserved[6];
    struct BootInfo_BlockLBA blocks[9];
}__attribute__((packed));

struct BootInfo_BlockList272 {
    uint32_t  jump;
    uint64_t  address;
	uint16_t  sector_size;
    char      reserved[6];
    struct BootInfo_BlockLBA blocks[21];
	char      string[240];
}__attribute__((packed));

struct BootInfo_BlockList512 {
	uint32_t  jump;
	uint64_t  address;
	uint16_t  sector_size;
    char      reserved[6];
	struct BootInfo_BlockLBA blocks[41];
}__attribute__((packed));

struct BootInfo_KernelInfo {
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
