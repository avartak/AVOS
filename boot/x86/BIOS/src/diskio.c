#include <boot/x86/BIOS/include/diskio.h>
#include <boot/x86/BIOS/include/bios.h>

uint8_t DiskIO_LowMemoryBuffer[0x1000];

bool DiskIO_CheckForBIOSExtensions(uint8_t drive) {

    struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax   = 0x4100;
    BIOS_regs.ebx   = 0x55AA;
    BIOS_regs.edx   = drive;

    BIOS_Interrupt(0x13, &BIOS_regs);

    if ((BIOS_regs.flags & 1) == 1 || BIOS_regs.ebx != 0xAA55 || (BIOS_regs.ecx & 1) == 0) return false;
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


size_t DiskIO_ReadFromDisk(uint8_t drive, uintptr_t mem_start_addr, uint64_t disk_start_sector, size_t num_sectors) {

	if (num_sectors == 0) return 0;

	size_t bytes_read = 0;
	struct DiskIO_Geometry geom;
	geom.size = 0x1A;
	if (!DiskIO_CheckForBIOSExtensions(drive)) return 0;
	if (!DiskIO_GetDiskGeometry(drive, &geom)) return 0;
	if (geom.size > 0x1000) return 0;

    struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);

	struct DiskIO_DAP dap;

	dap.size           = 0x10;
	dap.unused1        = 0;
	dap.sectors        = 1;
	dap.unused2        = 0;
	dap.memory_offset  = (uintptr_t)DiskIO_LowMemoryBuffer &  0xF;
	dap.memory_segment = (uintptr_t)DiskIO_LowMemoryBuffer >> 4;
	dap.start_sector   = disk_start_sector;

	uint8_t* dst = (uint8_t*)mem_start_addr;
	for (size_t i = 0; i < num_sectors; i++) {
		BIOS_regs.eax = 0x4200;
		BIOS_regs.edx = drive;
		BIOS_regs.esi = (uintptr_t)(&dap) &  0xF;
		BIOS_regs.ds  = (uintptr_t)(&dap) >> 4;

    	BIOS_Interrupt(0x13, &BIOS_regs);
		if ((BIOS_regs.flags & 1) == 1) break;
		for (size_t j = 0; j < geom.bytes_per_sector; j++) *(dst++) = DiskIO_LowMemoryBuffer[j];
		bytes_read += geom.bytes_per_sector;
		dap.start_sector++;
	}

    return bytes_read;
}

