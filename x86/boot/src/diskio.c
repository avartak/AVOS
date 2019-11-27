#include <x86/boot/include/diskio.h>
#include <x86/boot/include/bios.h>

uint8_t DiskIO_LowMemoryBuffer[512];

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

bool DiskIO_GetDiskGeometry(uint8_t drive, struct DiskIO_Geometry* geometry) {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax   = 0x4800;
    BIOS_regs.edx   = drive;
    BIOS_regs.esi   = (uintptr_t)geometry & 0xF;
    BIOS_regs.ds    = (uintptr_t)geometry >> 4;

    BIOS_Interrupt(0x13, &BIOS_regs);

    if ((BIOS_regs.flags & 1) == 1) return false;
    return true;
}


bool DiskIO_ReadUsingLBA(uint8_t drive, uintptr_t mem_start_addr, uint32_t disk_start_sector_lo, uint32_t disk_start_sector_hi, size_t num_sectors) {

	if (num_sectors == 0) return false;

    struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);

	struct DiskIO_DAP dap;

	dap.size = 0x10;
	dap.unused1 = 0;
	dap.sectors = 1;
	dap.unused2 = 0;
	dap.memory_offset   = (uintptr_t)DiskIO_LowMemoryBuffer & 0xF;
	dap.memory_segment  = (uintptr_t)DiskIO_LowMemoryBuffer >> 4;
	dap.start_sector_lo = disk_start_sector_lo;
	dap.start_sector_hi = disk_start_sector_hi;

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

size_t DiskIO_ReadFromDisk(uint8_t drive, uintptr_t mem_start_addr, uint32_t disk_start_sector_lo, uint32_t disk_start_sector_hi, size_t num_sectors) {
	bool read_success = false;

	struct DiskIO_Geometry geom;
	geom.size = 0x1E;
	if (!DiskIO_CheckForBIOSExtensions(drive)) read_success = false;
	if (!DiskIO_GetDiskGeometry(drive, &geom)) read_success = false;
	read_success = true;
	if (!read_success) return 0;

	read_success = DiskIO_ReadUsingLBA(drive, mem_start_addr, disk_start_sector_lo, disk_start_sector_hi, num_sectors);
	if (!read_success) return 0;
	return num_sectors * geom.bytes_per_sector;
}

