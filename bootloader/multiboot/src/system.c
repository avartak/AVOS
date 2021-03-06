#include <bootloader/multiboot/include/system.h>
#include <bootloader/multiboot/include/bios.h>
#include <bootloader/multiboot/include/multiboot.h>
#include <bootloader/multiboot/include/string.h>

uint32_t System_StoreAPMInfo(uint32_t addr) {

	// First check if APM is supported
	struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);
	
	BIOS_ClearRegistry(&BIOS_regs);
	BIOS_regs.ax = 0x5300;
	BIOS_Interrupt(0x15, &BIOS_regs);
	
	if ((BIOS_regs.flags & 1) == 1) return addr;
	if ((char)(BIOS_regs.bh) != 'P' || (char)(BIOS_regs.bl) != 'M') return addr;
	
	// If APM support is detected then save the APM version and flags
	uint16_t version = BIOS_regs.ax;
	uint16_t flags   = BIOS_regs.cx;
	
	// First disconnect from an APM interface
	BIOS_regs.ax = 0x5304;
	BIOS_Interrupt(0x15, &BIOS_regs);
	if ((BIOS_regs.flags & 1) == 1 || BIOS_regs.ah != 3) return addr;
	
	// Now connect to a 32-bit APM interface
	BIOS_ClearRegistry(&BIOS_regs);
	BIOS_regs.ax = 0x5303;
	BIOS_Interrupt(0x15, &BIOS_regs);
	if (BIOS_regs.flags & 1) return addr;
	
	/*
		 AX : Real mode segment denoting the base address for a 32-bit code segment expected by APM (shift left by 4 bits to get the actual base address)
		EBX : 32-bit entry point of the protected mode code
		 CX : Real mode segment denoting the base address for a 16-bit code segment expected by APM (shift left by 4 bits to get the actual base address)
		 DX : Real mode segment denoting the base address for a 32-bit data segment expected by APM (shift left by 4 bits to get the actual base address)
		ESI : Bits 0:15 give the 32-Bit code segment length ; bits 16:31 give the 16-Bit code segment length
		 DI : Data segment length
	*/
	
	// Save the APM interface information
	struct Multiboot_APM_Interface* apm_info = (struct Multiboot_APM_Interface*)addr;
	apm_info->version     =  version;
	apm_info->cseg        =  BIOS_regs.ax;
	apm_info->offset      =  BIOS_regs.ebx; 
	apm_info->cseg_16     =  BIOS_regs.cx;
	apm_info->dseg        =  BIOS_regs.dx;
	apm_info->flags       =  flags;
	apm_info->cseg_len    =  BIOS_regs.si;
	apm_info->cseg_16_len = (BIOS_regs.esi & 0xFFFF0000) >> 0x10;
	apm_info->dseg_len    =  BIOS_regs.di;
	
	return addr + 20;

}


uint32_t System_StoreSMBIOSInfo(uint32_t addr) {

	// Find the memory address of the SMBIOS entry point table in low memory --> It starts on a 16-byte aligned boundary in the range 0xF0000 - 0x100000
	uint32_t mem = 0xF0000;
	for (; mem < 0x100000; mem += 0x10) {
		char* str = (char*)mem;
		if (str[0] == '_' && str[1] == 'S' && str[2] == 'M' && str[3] == '_') {
			size_t length = (uint8_t)(str[5]);
			uint8_t checksum = 0;
			for (size_t i = 0; i < length; i++) checksum += (uint8_t)(str[i]);
			if (checksum == 0) break;
		}
	}
	if (mem == 0x100000) return addr;
	
	struct Multiboot_SMBIOS_EntryPointTable* smbios_ept = (struct Multiboot_SMBIOS_EntryPointTable*)mem;
	
	// First two bytes contain the version information; next 6 bytes are reserved	
	uint8_t* ver = (uint8_t*)addr;
	ver[0] = smbios_ept->major_version;
	ver[1] = smbios_ept->minor_version;
	for (size_t i = 2; i < 8; i++) ver[i] = 0;
	
	// Then we copy all the SMBIOS tables starting with the entry point table (not clear if the EPT should be included)
	uint8_t* src = (uint8_t*)mem;
	uint8_t* dst = ver + 8;
	memmove(dst, src, smbios_ept->length);
	
	// Now copy all the SMBIOS tables (the starting address and the total size of all tables is in the EPT)
	src  = (uint8_t*)smbios_ept->table_address;
	dst += smbios_ept->length;
	memmove(dst, src, smbios_ept->table_length);
	
	// Add 0-padding to keep the MBI 8-byte aligned
	uint32_t retval = (uint32_t)dst + smbios_ept->table_length;
	if (retval % 8 != 0) {
		memset((uint8_t*)retval, 0, 8 - (retval % 8));
		retval += 8 - (retval % 8);
	}
	
	return retval;

}

uint32_t System_StoreACPIInfo(uint32_t addr, bool old) {

	// The Root System Description Pointer (RSDP) can be found either in the first KB of the EBDA (the 16-bit segment pointing to the EBDA is located ar 0x40E)
	// Or it can be found in the range 0xE0000 - 0x100000
	bool found = false;
	uint32_t ptr = (uint32_t)(((uint16_t*)0x40E)[0]) << 4;
	uint32_t ebda_1kb = ptr + 0x400;
	for (; ptr < 0x100000; ptr = (ptr + 0x10 == ebda_1kb ? 0xE0000 : ptr + 0x10)) {
		if (((uint8_t*)ptr)[0] != 'R') continue;
		if (((uint8_t*)ptr)[1] != 'S') continue;
		if (((uint8_t*)ptr)[2] != 'D') continue;
		if (((uint8_t*)ptr)[3] != ' ') continue;
		if (((uint8_t*)ptr)[4] != 'P') continue;
		if (((uint8_t*)ptr)[5] != 'T') continue;
		if (((uint8_t*)ptr)[6] != 'R') continue;
		if (((uint8_t*)ptr)[7] != ' ') continue;

		struct Multiboot_RSDPv1* rsdp_v1 = (struct Multiboot_RSDPv1*)ptr;
		
		uint32_t sum1 = 0; // Checksum for (old) RSDP v1
		uint32_t sum2 = 0; // Checksum for (new) RSDP v2
		
		for (size_t i = 0; i < sizeof(struct Multiboot_RSDPv1); i++) sum1 += ((uint8_t*)ptr)[i];
		if ((sum1 & 0xFF) != 0) continue;
		if (!old) {
			if (rsdp_v1->revision != 2) continue;
			for (size_t i = sizeof(struct Multiboot_RSDPv1); i < sizeof(struct Multiboot_RSDPv2) - sizeof(struct Multiboot_RSDPv1); i++) sum2 += ((uint8_t*)ptr)[i];
		}
		if ((sum2 & 0xFF) != 0) continue;

		found = true;
		break;
	}

	if (!found) return addr;
	else {
		uint8_t* src = (uint8_t*)ptr;
		uint8_t* dst = (uint8_t*)addr;
		size_t   len = (old ? sizeof(struct Multiboot_RSDPv1) : sizeof(struct Multiboot_RSDPv2));
		
		memmove(dst, src, len);
		return addr + len;
	}

}
