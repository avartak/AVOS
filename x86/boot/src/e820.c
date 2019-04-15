#include <x86/boot/include/bios.h>
#include <x86/boot/include/e820.h>

size_t E820_Table_size = 0;
struct E820_Table_Entry* E820_Table = MEMORY_NULL_PTR;

uintptr_t E820_StoreMap(uintptr_t addr) {

	E820_Table = (struct E820_Table_Entry*)addr;
	struct E820_Table_Entry* current_entry = E820_Table;

	if (addr > 0xFFFFF) return 0;

	struct BIOS_Registers BIOS_regs;

	BIOS_regs.eax   = 0x00000000;
	BIOS_regs.ebx   = 0x00000000;			
	BIOS_regs.ecx   = 0x00000000;
	BIOS_regs.edx   = 0x00000000;
	BIOS_regs.esi   = 0x00000000;			
	BIOS_regs.edi   = (addr & 0xF);
	BIOS_regs.ebp   = 0x00000000;			
	BIOS_regs.esp   = 0x00000000;			
	BIOS_regs.ds    = 0x0000;			
	BIOS_regs.es    = (addr >> 4);			
	BIOS_regs.ss    = 0x0000;			
	BIOS_regs.flags = 0x0000;

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

	E820_Table_size = BIOS_regs.esi;
	return addr + BIOS_regs.esi;

}
