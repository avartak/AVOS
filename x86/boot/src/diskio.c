#include <x86/boot/include/diskio.h>
#include <x86/boot/include/bios.h>

bool DiskIO_GetGeometry(uint8_t drive, struct DiskIO_Geometry* geometry) {

    struct BIOS_Registers BIOS_regs;

    BIOS_regs.eax   = 0x00000800;
    BIOS_regs.ebx   = 0;
    BIOS_regs.ecx   = 0;
    BIOS_regs.edx   = drive;
    BIOS_regs.esi   = 0;
    BIOS_regs.edi   = 0;
    BIOS_regs.ebp   = 0;
    BIOS_regs.esp   = 0;
    BIOS_regs.ds    = 0;
    BIOS_regs.es    = 0;
    BIOS_regs.ss    = 0;
    BIOS_regs.flags = 0;

	BIOS_Interrupt(0x13, &BIOS_regs);

	if ((BIOS_regs.flags & 1) == 1) return false;

	geometry->heads = ((BIOS_regs.edx & 0x0000FF00) >> 8) + 1;
	geometry->sectors_per_track = BIOS_regs.ecx & 0x3FF;
	geometry->sectors_per_cylinder = geometry->heads * geometry->sectors_per_track;

	return true;
}

bool DiskIO_CheckForBIOSExtensions(uint8_t drive) {

    struct BIOS_Registers BIOS_regs;

    BIOS_regs.eax   = 0x00004100;
    BIOS_regs.ebx   = 0x000055AA;
    BIOS_regs.ecx   = 0;
    BIOS_regs.edx   = drive;
    BIOS_regs.esi   = 0;
    BIOS_regs.edi   = 0;
    BIOS_regs.ebp   = 0;
    BIOS_regs.esp   = 0;
    BIOS_regs.ds    = 0;
    BIOS_regs.es    = 0;
    BIOS_regs.ss    = 0;
    BIOS_regs.flags = 0;

    BIOS_Interrupt(0x13, &BIOS_regs);

    if (BIOS_regs.ebx != 0xAA55 || (BIOS_regs.ecx & 1) == 0 || (BIOS_regs.flags & 1) == 1) return false;

    return true;
}

bool DiskIO_ReadUsingLBA(uint8_t drive, uintptr_t scratch, uintptr_t kernel_start, uint32_t kernel_disk_start_lo, uint32_t kernel_disk_start_hi, size_t kernel_size) {

	if (!DiskIO_CheckForBIOSExtensions(drive)) return false;
	if (scratch > 0xFFFFF) return false;
	if (kernel_size == 0) return false;

    struct BIOS_Registers BIOS_regs;
	struct DiskIO_DAP dap;

    BIOS_regs.eax   = 0;
    BIOS_regs.ebx   = 0;
    BIOS_regs.ecx   = 0;
    BIOS_regs.edx   = 0;
    BIOS_regs.esi   = 0;
    BIOS_regs.edi   = 0;
    BIOS_regs.ebp   = 0;
    BIOS_regs.esp   = 0;
    BIOS_regs.ds    = 0;
    BIOS_regs.es    = 0;
    BIOS_regs.ss    = 0;
    BIOS_regs.flags = 0;

	dap.size = 0x10;
	dap.unused1 = 0;
	dap.sectors = 1;
	dap.unused2 = 0;
	dap.memory_offset   = scratch & 0xF;
	dap.memory_segment  = scratch >> 4;
	dap.start_sector_lo = kernel_disk_start_lo;
	dap.start_sector_hi = kernel_disk_start_hi;

	uintptr_t kernel_pos = kernel_start;

	for (size_t i = 0; i < kernel_size; i++) {
		BIOS_regs.eax = 0x00004200;
		BIOS_regs.edx = drive;
		BIOS_regs.esi = (uintptr_t)(&dap) & 0xF;
		BIOS_regs.ds  = (uintptr_t)(&dap) >> 4;

    	BIOS_Interrupt(0x13, &BIOS_regs);
		if ((BIOS_regs.flags & 1) == 1) break;
		uint8_t* src = (uint8_t*)scratch;
		uint8_t* dst = (uint8_t*)kernel_pos;
		for (size_t j = 0; j < 0x200; j++) dst[j] = src[j];

		dap.start_sector_lo++;
		if (dap.start_sector_lo == 0) dap.start_sector_hi++;
		kernel_pos += 0x200;
	}

    return true;
}

bool DiskIO_ReadUsingCHS(uint8_t drive, uintptr_t scratch, uintptr_t kernel_start, uint32_t kernel_disk_start_lo, uint32_t kernel_disk_start_hi, size_t kernel_size) {

    if (scratch > 0xFFFFF) return false;
    if (kernel_size == 0) return false;
    if (kernel_disk_start_hi > 0) return false;
	if (0xFFFFFFFF - kernel_disk_start_lo < kernel_size) return false;

	struct DiskIO_Geometry geometry;
	if (!DiskIO_GetGeometry(drive, &geometry)) return false;

    struct BIOS_Registers BIOS_regs;

    BIOS_regs.eax   = 0;
    BIOS_regs.ebx   = 0;
    BIOS_regs.ecx   = 0;
    BIOS_regs.edx   = 0;
    BIOS_regs.esi   = 0;
    BIOS_regs.edi   = 0;
    BIOS_regs.ebp   = 0;
    BIOS_regs.esp   = 0;
    BIOS_regs.ds    = 0;
    BIOS_regs.es    = 0;
    BIOS_regs.ss    = 0;
    BIOS_regs.flags = 0;

    uintptr_t kernel_pos  = kernel_start;
	uint32_t  read_sector = kernel_disk_start_lo;

    for (size_t i = 0; i < kernel_size; i++) {
        BIOS_regs.eax = 0x00000201;
        BIOS_regs.ebx = scratch & 0xF;
        BIOS_regs.edx = drive;
        BIOS_regs.es  = scratch >> 4;

		uint16_t sector   = (read_sector % geometry.sectors_per_track) + 1;
		uint16_t cylinder =  read_sector / geometry.sectors_per_cylinder;
		uint16_t head     = (read_sector / geometry.sectors_per_track) % geometry.heads;

		if (head > 255 || cylinder > 1023 || sector > 63) return false;

		BIOS_regs.ecx  = (cylinder << 6) + sector;
		BIOS_regs.edx +=  head << 8;

        BIOS_Interrupt(0x13, &BIOS_regs);
        if ((BIOS_regs.flags & 1) == 1 || (BIOS_regs.eax & 0xFF) != 1) break;
        uint8_t* src = (uint8_t*)scratch;
        uint8_t* dst = (uint8_t*)kernel_pos;
        for (size_t j = 0; j < 0x200; j++) dst[j] = src[j];

        kernel_pos += 0x200;
		read_sector++;
    }

    return true;
}

bool DiskIO_ReadFromDisk(uint8_t drive, uintptr_t scratch, uintptr_t kernel_start, uint32_t kernel_disk_start_lo, uint32_t kernel_disk_start_hi, size_t kernel_size) {
	bool lba_worked =       DiskIO_ReadUsingLBA(drive, scratch, kernel_start, kernel_disk_start_lo, kernel_disk_start_hi, kernel_size);
	if (!lba_worked) return DiskIO_ReadUsingCHS(drive, scratch, kernel_start, kernel_disk_start_lo, kernel_disk_start_hi, kernel_size);	
	else return true;
}

