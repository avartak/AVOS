#include <bootloader/multiboot/include/diskio.h>
#include <bootloader/multiboot/include/bios.h>

uint8_t DiskIO_LowMemoryBuffer[DISKIO_MAX_SECTOR_SIZE];

bool DiskIO_CheckForBIOSExtensions(uint8_t drive) {

	struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);

	BIOS_regs.ax = 0x4100;
	BIOS_regs.bx = 0x55AA;
	BIOS_regs.dx = drive;
	
	BIOS_Interrupt(0x13, &BIOS_regs);
	
	if ((BIOS_regs.flags & 1) == 1 || BIOS_regs.bx != 0xAA55 || (BIOS_regs.cx & 1) == 0) return false;
	return true;
}

bool DiskIO_GetDiskGeometry(uint8_t drive, struct DiskIO_Geometry* geometry) {

	struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);
	
	BIOS_regs.ax = 0x4800;
	BIOS_regs.dx = drive;
	BIOS_regs.si = (uint32_t)geometry & 0xF;
	BIOS_regs.ds = (uint32_t)geometry >> 4;
	
	BIOS_Interrupt(0x13, &BIOS_regs);
	
	if ((BIOS_regs.flags & 1) == 1) return false;
	return true;
}


uint32_t DiskIO_ReadFromDisk(uint8_t drive, uint32_t mem_start_addr, uint64_t disk_start_sector, uint32_t num_sectors) {

	if (num_sectors == 0) return 0;
	
	struct DiskIO_Geometry geom;
	geom.size = DISKIO_DISK_GEOMETRY_PACKET_SIZE;
	if (!DiskIO_CheckForBIOSExtensions(drive)) return 0;
	if (!DiskIO_GetDiskGeometry(drive, &geom)) return 0;
	if (geom.size > DISKIO_MAX_SECTOR_SIZE)    return 0;
	
	struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);
	
	struct DiskIO_DAP dap;
	dap.size           = DISKIO_DISK_ADDRESS_PACKET_SIZE;
	dap.unused1        = 0;
	dap.sectors        = 1;
	dap.unused2        = 0;
	dap.memory_offset  = (uint32_t)DiskIO_LowMemoryBuffer &  0xF;
	dap.memory_segment = (uint32_t)DiskIO_LowMemoryBuffer >> 4;
	dap.start_sector   = disk_start_sector;
	
	uint32_t bytes_read = 0;
	uint8_t* dst = (uint8_t*)mem_start_addr;
	for (uint32_t i = 0; i < num_sectors; i++) {
		BIOS_regs.ax = 0x4200;
		BIOS_regs.dx = drive;
		BIOS_regs.si = (uint32_t)(&dap) &  0xF;
		BIOS_regs.ds = (uint32_t)(&dap) >> 4;
		
		BIOS_Interrupt(0x13, &BIOS_regs);
		if ((BIOS_regs.flags & 1) == 1) break;
		for (uint32_t j = 0; j < geom.bytes_per_sector; j++) *(dst++) = DiskIO_LowMemoryBuffer[j];
		bytes_read += geom.bytes_per_sector;
		dap.start_sector++;
	}
	
	return bytes_read;
}

