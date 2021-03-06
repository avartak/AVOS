#ifndef BOOTLOADER_DISKIO_H
#define BOOTLOADER_DISKIO_H

#include <stdint.h>
#include <stdbool.h>

#define DISKIO_MAX_SECTOR_SIZE             0x1000
#define DISKIO_DISK_ADDRESS_PACKET_SIZE    0x10
#define DISKIO_DISK_GEOMETRY_PACKET_SIZE   0x1A

struct DiskIO_Geometry {
	uint16_t  size;
	uint16_t  flags;
	uint32_t  cylinders;
	uint32_t  heads;
	uint32_t  sectors_per_track;
	uint64_t  sectors;
	uint16_t  bytes_per_sector;
	uintptr_t dpte_ptr; 
}__attribute__((packed));

struct DiskIO_DAP {
	uint8_t  size;
	uint8_t  unused1;
	uint8_t  sectors;
	uint8_t  unused2;
	uint16_t memory_offset;
	uint16_t memory_segment;
	uint64_t start_sector;
}__attribute__((packed));

extern uint8_t  DiskIO_LowMemoryBuffer[];

extern bool     DiskIO_CheckForBIOSExtensions(uint8_t drive);
extern bool     DiskIO_GetDiskGeometry       (uint8_t drive, struct DiskIO_Geometry* geometry);
extern uint32_t DiskIO_ReadFromDisk          (uint8_t drive, uint32_t mem_start_addr, uint64_t disk_start_sector, uint32_t num_sectors);

#endif
