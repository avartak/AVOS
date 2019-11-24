#ifndef X86_BOOT_DISKIO_H
#define X86_BOOT_DISKIO_H

#include <kernel/include/common.h>

#define DISKIO_SECTOR_SIZE 0x200

struct DiskIO_DAP {
	uint8_t  size;
	uint8_t  unused1;
	uint8_t  sectors;
	uint8_t  unused2;
	uint16_t memory_offset;
	uint16_t memory_segment;
	uint32_t start_sector_lo;
	uint32_t start_sector_hi;
}__attribute__((packed));

struct DiskIO_Geometry {
    uint8_t  heads;
    uint8_t  sectors_per_track;
    uint16_t sectors_per_cylinder;
}__attribute__((packed));

extern uint8_t DiskIO_LowMemoryBuffer[];

extern bool   DiskIO_GetCHSGeometry(uint8_t drive, struct DiskIO_Geometry* geometry);
extern bool   DiskIO_CheckForBIOSExtensions(uint8_t drive);
extern bool   DiskIO_ReadUsingLBA(uint8_t drive, uintptr_t kernel_start, uint32_t kernel_disk_start_lo, uint32_t kernel_disk_start_hi, size_t kernel_size);
extern bool   DiskIO_ReadUsingCHS(uint8_t drive, uintptr_t kernel_start, uint32_t kernel_disk_start_lo, uint32_t kernel_disk_start_hi, size_t kernel_size);
extern size_t DiskIO_ReadFromDisk(uint8_t drive, uintptr_t kernel_start, uint32_t kernel_disk_start_lo, uint32_t kernel_disk_start_hi, size_t kernel_size);


#endif
