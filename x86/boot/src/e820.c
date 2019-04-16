#include <x86/boot/include/bios.h>
#include <x86/boot/include/e820.h>

uintptr_t E820_StoreInfo(uintptr_t addr) {

	struct E820_Table_Entry* current_entry = (struct E820_Table_Entry*)addr;

	if (addr > 0xA0000) return 0;

	struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);

	BIOS_regs.edi   = (addr & 0xF);
	BIOS_regs.es    = (addr >> 4);			

	while (true) {
		BIOS_regs.eax = 0x0000E820;
		BIOS_regs.ecx = 0x00000018;
		BIOS_regs.edx = 0x534D4150;
		current_entry->acpi3 = 1;

		BIOS_Interrupt(0x15, &BIOS_regs);

		if (BIOS_regs.eax != 0x534D4150 || BIOS_regs.ebx == 0 || (BIOS_regs.flags & 1) == 1) break;

	    if ((BIOS_regs.ecx <= 20 || current_entry->acpi3 == 1) && current_entry->size > 0) {
    	    BIOS_regs.esi += 0x18;
        	BIOS_regs.edi += 0x18;
        	current_entry++;
    	}
	}

	if (BIOS_regs.esi == 0) return 0;

	return addr + BIOS_regs.esi;

}
