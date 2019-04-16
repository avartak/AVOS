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

bool DiskIO_ReadUsingLBA(uint8_t drive, uintptr_t kernel_start, uint32_t kernel_disk_start, size_t kernel_size) {

	if (!DiskIO_CheckForBIOSExtensions(drive)) return false;
	if (kernel_size == 0) return false;

    struct BIOS_Registers BIOS_regs;
	struct DiskIO_DAP dap;

	BIOS_ClearRegistry(&BIOS_regs);

	dap.size = 0x10;
	dap.unused1 = 0;
	dap.sectors = 1;
	dap.unused2 = 0;
	dap.memory_offset   = SCRATCH & 0xF;
	dap.memory_segment  = SCRATCH >> 4;
	dap.start_sector_lo = kernel_disk_start;
	dap.start_sector_hi = 0;

	uintptr_t kernel_pos = kernel_start;

	for (size_t i = 0; i < kernel_size; i++) {
		BIOS_regs.eax = 0x4200;
		BIOS_regs.edx = drive;
		BIOS_regs.esi = (uintptr_t)(&dap) & 0xF;
		BIOS_regs.ds  = (uintptr_t)(&dap) >> 4;

    	BIOS_Interrupt(0x13, &BIOS_regs);
		if ((BIOS_regs.flags & 1) == 1) break;
		uint8_t* src = (uint8_t*)SCRATCH;
		uint8_t* dst = (uint8_t*)kernel_pos;
		for (size_t j = 0; j < 0x200; j++) dst[j] = src[j];

		dap.start_sector_lo++;
		if (dap.start_sector_lo == 0) dap.start_sector_hi++;
		kernel_pos += 0x200;
	}

    return true;
}

bool DiskIO_ReadUsingCHS(uint8_t drive, uintptr_t kernel_start, uint32_t kernel_disk_start, size_t kernel_size) {

    if (kernel_size == 0) return false;
	if (0xFFFFFFFF - kernel_disk_start < kernel_size) return false;

	struct DiskIO_Geometry geometry;
	if (!DiskIO_GetGeometry(drive, &geometry)) return false;

    struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);

    uintptr_t kernel_pos  = kernel_start;
	uint32_t  read_sector = kernel_disk_start;

    for (size_t i = 0; i < kernel_size; i++) {
        BIOS_regs.eax = 0x0201;
        BIOS_regs.ebx = SCRATCH & 0xF;
        BIOS_regs.edx = drive;
        BIOS_regs.es  = SCRATCH >> 4;

		uint16_t sector   = (read_sector % geometry.sectors_per_track) + 1;
		uint16_t cylinder =  read_sector / geometry.sectors_per_cylinder;
		uint16_t head     = (read_sector / geometry.sectors_per_track) % geometry.heads;

		if (head > 255 || cylinder > 1023 || sector > 63) return false;

		BIOS_regs.ecx  = (cylinder << 6) + sector;
		BIOS_regs.edx +=  head << 8;

        BIOS_Interrupt(0x13, &BIOS_regs);
        if ((BIOS_regs.flags & 1) == 1 || (BIOS_regs.eax & 0xFF) != 1) break;
        uint8_t* src = (uint8_t*)SCRATCH;
        uint8_t* dst = (uint8_t*)kernel_pos;
        for (size_t j = 0; j < 0x200; j++) dst[j] = src[j];

        kernel_pos += 0x200;
		read_sector++;
    }

    return true;
}

bool DiskIO_ReadFromDisk(uint8_t drive, uintptr_t kernel_start, uint32_t kernel_disk_start, size_t kernel_size) {
	bool lba_worked =       DiskIO_ReadUsingLBA(drive, kernel_start, kernel_disk_start, kernel_size);
	if (!lba_worked) return DiskIO_ReadUsingCHS(drive, kernel_start, kernel_disk_start, kernel_size);	
	else return true;
}

