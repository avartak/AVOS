#include <x86/boot/include/discovery.h>
#include <x86/boot/include/multiboot.h>
#include <x86/boot/include/bios.h>

uintptr_t Discovery_StoreAPMInfo(uintptr_t addr) {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

	BIOS_regs.eax = 0x00005304;
    BIOS_Interrupt(0x15, &BIOS_regs);
    if ((BIOS_regs.flags & 1) == 1 && (BIOS_regs.eax & 0xFF00 >> 8) != 3) return addr;

    BIOS_ClearRegistry(&BIOS_regs);
    BIOS_regs.eax = 0x00005300;
    BIOS_Interrupt(0x15, &BIOS_regs);

    if ((BIOS_regs.flags & 1) == 1) return addr;
	if ((char)((BIOS_regs.ebx & 0xFF00) >> 8) != 'P' || (char)(BIOS_regs.ebx & 0xFF) != 'M') return addr;

	uint16_t version = BIOS_regs.eax & 0xFFFF;
	uint16_t flags   = BIOS_regs.ecx & 0xFFFF;

    BIOS_ClearRegistry(&BIOS_regs);
    BIOS_regs.eax = 0x00005303;
    BIOS_Interrupt(0x15, &BIOS_regs);
    if ((BIOS_regs.flags & 1) == 1) return addr;

	uint16_t* info = (uint16_t*)addr;
	info[0]  = version;
	info[1]  = BIOS_regs.eax & 0xFFFF;
	info[2]  = BIOS_regs.ebx & 0xFFFF;
	info[3]  = (BIOS_regs.ebx & 0xFFFF0000) >> 16;
	info[4]  = BIOS_regs.ecx & 0xFFFF;
	info[5]  = BIOS_regs.edx & 0xFFFF;
	info[6]  = flags;
	info[7]  = BIOS_regs.esi & 0xFFFF;
	info[8]  = (BIOS_regs.esi & 0xFFFF0000) >> 16;
	info[9]  = BIOS_regs.edi & 0xFFFF;
	info[10] = 0;
	info[11] = 0;

	return addr + 24;

}


uintptr_t Discovery_StoreSMBIOSInfo(uintptr_t addr) {

    uintptr_t mem;
    for (mem = 0xF0000; mem < 0x100000; mem += 16) {
        char* str = (char*)mem;
        if (str[0] == '_' && str[1] == 'S' && str[2] == 'M' && str[3] == '_') {
            size_t length = (uint8_t)(str[5]);
            uint8_t checksum = 0;
            for (size_t i = 0; i < length; i++) checksum += (uint8_t)(str[i]);
            if (checksum == 0) break;
        }
    }

    if (mem == 0x100000) return addr;
    else {
        struct Multiboot_SMBIOS_EntryPointTable* smbios_ept = (struct Multiboot_SMBIOS_EntryPointTable*)mem;

		uint8_t* rev = (uint8_t*)addr;
		rev[0] = smbios_ept->major_version;
		rev[1] = smbios_ept->minor_version;
		for (size_t i = 2; i < 8; i++) rev[i] = 0;

        uint8_t* src = (uint8_t*)mem;
        uint8_t* dst = rev + 8;

        memmove(dst, src, smbios_ept->table_length);
		uintptr_t retval = addr + 8 + smbios_ept->table_length;

		if (retval % 8 != 0) {
			memset((uint8_t*)retval, 0, 8 - (retval % 8));
			retval += 8 - (retval % 8);
		}

        return retval;
    }

}

uintptr_t Discovery_StoreACPIInfo(uintptr_t addr, bool old) {

    bool found = false;
    uintptr_t ptr;
    for (ptr = 0x80000; ptr < 0xA0000; ptr = ptr + 0x10) {
        if (((uint8_t*)ptr)[0] != 'R') continue;
        if (((uint8_t*)ptr)[1] != 'S') continue;
        if (((uint8_t*)ptr)[2] != 'D') continue;
        if (((uint8_t*)ptr)[3] != ' ') continue;
        if (((uint8_t*)ptr)[4] != 'P') continue;
        if (((uint8_t*)ptr)[5] != 'T') continue;
        if (((uint8_t*)ptr)[6] != 'R') continue;
        if (((uint8_t*)ptr)[7] != ' ') continue;

        struct Multiboot_RSDPv1* rsdp_v1 = (struct Multiboot_RSDPv1*)ptr;

        uint32_t sum1 = 0;
        uint32_t sum2 = 0;

        for (size_t i = 0; i < sizeof(struct Multiboot_RSDPv1); i++) sum1 += ((uint8_t*)ptr)[i];
        if ((sum1 & 0xF) != 0) continue;
        if (!old) {
            if (rsdp_v1->revision != 2) continue;
            for (size_t i = sizeof(struct Multiboot_RSDPv1); i < sizeof(struct Multiboot_RSDPv2) - sizeof(struct Multiboot_RSDPv1); i++) sum2 += ((uint8_t*)ptr)[i];
        }
        if ((sum2 & 0xF) != 0) continue;

        found = true;
        break;
    }

    if (!found) {
    for (ptr = 0xE0000; ptr < 0x100000; ptr = ptr + 0x10) {
        if (((uint8_t*)ptr)[0] != 'R') continue;
        if (((uint8_t*)ptr)[1] != 'S') continue;
        if (((uint8_t*)ptr)[2] != 'D') continue;
        if (((uint8_t*)ptr)[3] != ' ') continue;
        if (((uint8_t*)ptr)[4] != 'P') continue;
        if (((uint8_t*)ptr)[5] != 'T') continue;
        if (((uint8_t*)ptr)[6] != 'R') continue;
        if (((uint8_t*)ptr)[7] != ' ') continue;

        struct Multiboot_RSDPv1* rsdp_v1 = (struct Multiboot_RSDPv1*)ptr;

        uint32_t sum1 = 0;
        uint32_t sum2 = 0;

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
