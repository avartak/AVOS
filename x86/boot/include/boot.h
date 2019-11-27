#ifndef X86_BOOT_BLOCKLIST_H
#define X86_BOOT_BLOCKLIST_H

#include <kernel/include/common.h>

#define BOOT_BLOCKLIST_MAXBLOCKS128 9
#define BOOT_BLOCKLIST_MAXBLOCKS512 41
#define BOOT_BLOCKLIST_MAXBLOCKS272 21

struct Boot_Block {
	uint64_t  lba;
	size_t    num_sectors;
}__attribute__((packed));

struct Boot_BlockList128 {
    uint32_t  jump;
    uint32_t  load_address_lo;
    uint32_t  load_address_hi;
	uint16_t  sector_size;
    char      reserved[6];
    struct Boot_Block blocks[9];
}__attribute__((packed));

struct Boot_BlockList272 {
    uint32_t  jump;
    uint32_t  load_address_lo;
    uint32_t  load_address_hi;
	uint16_t  sector_size;
    char      reserved[6];
    struct Boot_Block blocks[21];
	char      string[240];
}__attribute__((packed));

struct Boot_BlockList512 {
	uint32_t  jump;
	uint32_t  load_address_lo;
	uint32_t  load_address_hi;
	uint16_t  sector_size;
    char      reserved[6];
	struct Boot_Block blocks[41];
}__attribute__((packed));

struct Boot_Kernel_Info {
    uint32_t  boot_drive_ID;
    uint32_t  boot_partition;
    uintptr_t part_info_ptr;
    uintptr_t blocklist_ptr;
    uintptr_t start;
    uintptr_t multiboot_header;
    uintptr_t entry;
    size_t    size;
}__attribute__((packed));


#endif
