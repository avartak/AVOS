#include <x86/boot/include/diskio.h>
#include <x86/boot/include/bios.h>

bool DiskIO_GetGeometry(uint8_t drive, struct DiskIO_Geometry* geometry) {

    struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax   = 0x0800;
    BIOS_regs.edx   = drive;

	BIOS_Interrupt(0x13, &BIOS_regs);

	if ((BIOS_regs.flags & 1) == 1) return false;

	geometry->heads = ((BIOS_regs.edx & 0x0000FF00) >> 8) + 1;
	geometry->sectors_per_track = BIOS_regs.ecx & 0x3F;
	geometry->sectors_per_cylinder = geometry->heads * geometry->sectors_per_track;

	return true;
}

bool DiskIO_CheckForBIOSExtensions(uint8_t drive) {

    struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax   = 0x4100;
    BIOS_regs.ebx   = 0x55AA;
    BIOS_regs.edx   = drive;

    BIOS_Interrupt(0x13, &BIOS_regs);

    if (BIOS_regs.ebx != 0xAA55 || (BIOS_regs.ecx & 1) == 0 || (BIOS_regs.flags & 1) == 1) return false;

    return true;
}

bool DiskIO_ReadUsingLBA(uint8_t drive, uintptr_t mem_start_addr, uint32_t disk_start_sector, size_t num_sectors) {

	if (!DiskIO_CheckForBIOSExtensions(drive)) return false;
	if (num_sectors == 0) return false;

    struct BIOS_Registers BIOS_regs;
	struct DiskIO_DAP dap;

	BIOS_ClearRegistry(&BIOS_regs);

	dap.size = 0x10;
	dap.unused1 = 0;
	dap.sectors = 1;
	dap.unused2 = 0;
	dap.memory_offset   = (uintptr_t)DiskIO_LowMemoryBuffer & 0xF;
	dap.memory_segment  = (uintptr_t)DiskIO_LowMemoryBuffer >> 4;
	dap.start_sector_lo = disk_start_sector;
	dap.start_sector_hi = 0;

	uintptr_t mem_pos = mem_start_addr;

	for (size_t i = 0; i < num_sectors; i++) {
		BIOS_regs.eax = 0x4200;
		BIOS_regs.edx = drive;
		BIOS_regs.esi = (uintptr_t)(&dap) & 0xF;
		BIOS_regs.ds  = (uintptr_t)(&dap) >> 4;

    	BIOS_Interrupt(0x13, &BIOS_regs);
		if ((BIOS_regs.flags & 1) == 1) break;
		uint8_t* src = DiskIO_LowMemoryBuffer;
		uint8_t* dst = (uint8_t*)mem_pos;
		for (size_t j = 0; j < 0x200; j++) dst[j] = src[j];

		dap.start_sector_lo++;
		if (dap.start_sector_lo == 0) dap.start_sector_hi++;
		mem_pos += 0x200;
	}

    return true;
}

bool DiskIO_ReadUsingCHS(uint8_t drive, uintptr_t mem_start_addr, uint32_t disk_start_sector, size_t num_sectors) {

    if (num_sectors == 0) return false;
	if (0xFFFFFFFF - disk_start_sector < num_sectors) return false;

	struct DiskIO_Geometry geometry;
	if (!DiskIO_GetGeometry(drive, &geometry)) return false;

    struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);

    uintptr_t mem_pos  = mem_start_addr;
	uint32_t  read_sector = disk_start_sector;

    for (size_t i = 0; i < num_sectors; i++) {
        BIOS_regs.eax = 0x0201;
        BIOS_regs.ebx = (uintptr_t)DiskIO_LowMemoryBuffer & 0xF;
        BIOS_regs.edx = drive;
        BIOS_regs.es  = (uintptr_t)DiskIO_LowMemoryBuffer >> 4;

		uint16_t sector   = (read_sector % geometry.sectors_per_track) + 1;
		uint16_t cylinder =  read_sector / geometry.sectors_per_cylinder;
		uint16_t head     = (read_sector / geometry.sectors_per_track) % geometry.heads;

		if (head > 255 || cylinder > 1023 || sector > 63) return false;

		BIOS_regs.ecx  = (cylinder << 6) + sector;
		BIOS_regs.edx +=  head << 8;

        BIOS_Interrupt(0x13, &BIOS_regs);
        if ((BIOS_regs.flags & 1) == 1 || (BIOS_regs.eax & 0xFF) != 1) break;
        uint8_t* src = DiskIO_LowMemoryBuffer;
        uint8_t* dst = (uint8_t*)mem_pos;
        for (size_t j = 0; j < 0x200; j++) dst[j] = src[j];

        mem_pos += 0x200;
		read_sector++;
    }

    return true;
}

size_t DiskIO_ReadFromDisk(uint8_t drive, uintptr_t mem_start_addr, uint32_t disk_start_sector, size_t num_sectors) {
	bool lba_worked = DiskIO_ReadUsingLBA(drive, mem_start_addr, disk_start_sector, num_sectors);
	if (!lba_worked) {
		lba_worked  = DiskIO_ReadUsingCHS(drive, mem_start_addr, disk_start_sector, num_sectors);
	}
	if (!lba_worked) return 0;
	return num_sectors * DISKIO_SECTOR_SIZE;
}
