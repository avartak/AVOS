#include <x86/boot/include/ram.h>
#include <x86/boot/include/multiboot.h>
#include <x86/boot/include/bios.h>

uintptr_t RAM_StoreBasicInfo(uintptr_t addr) {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

	BIOS_Interrupt(0x12, &BIOS_regs);
	if ((BIOS_regs.flags & 1) == 1) return addr;
	size_t memory_lower = BIOS_regs.eax & 0xFFFF;
	size_t memory_upper = 0;

	BIOS_ClearRegistry(&BIOS_regs);
	BIOS_regs.eax = 0xE801;
	BIOS_Interrupt(0x15, &BIOS_regs);
	if ((BIOS_regs.flags & 1) == 1 || (BIOS_regs.eax & 0xFF00) == 0x8600 || (BIOS_regs.eax & 0xFF00) == 0x8000) return addr;
	if ((BIOS_regs.eax & 0xFFFF) != 0 || (BIOS_regs.ecx & 0xFFFF) != 0) {
		if ((BIOS_regs.ecx & 0xFFFF) != 0) memory_upper += (BIOS_regs.ecx & 0xFFFF) + (BIOS_regs.edx & 0xFFFF)*0x40;
		else memory_upper += (BIOS_regs.eax & 0xFFFF) + (BIOS_regs.ebx & 0xFFFF)*0x40;
	}

	else {
		BIOS_ClearRegistry(&BIOS_regs);
		BIOS_regs.eax = 0x88;
		BIOS_Interrupt(0x15, &BIOS_regs);

		if ((BIOS_regs.flags & 1) == 1) return addr;
		memory_upper = (BIOS_regs.eax & 0xFFFF);
	}

	uint32_t* mem = (uint32_t*)(addr);
	mem[0] = memory_lower;
	mem[1] = memory_upper;
	
	if (mem[1] != 0) return addr + 8;
	else return addr;

}

uintptr_t RAM_StoreE820Info(uintptr_t addr) {

    struct Multiboot_E820_Entry* current_entry = (struct Multiboot_E820_Entry*)addr;

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.edi = (addr & 0xF);
    BIOS_regs.es  = (addr >> 4);

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

    return addr + BIOS_regs.esi;

}


uintptr_t RAM_MaxPresentMemoryAddress(struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size) {

	uintptr_t mem_max = 0;
	if (E820_Table_size == 0 || E820_Table == MEMORY_NULL_PTR) return mem_max;
	for (size_t i = 0; i < E820_Table_size; i++) {
		if (E820_Table[i].acpi3 != MULTIBOOT_MEMORY_ACPI3_FLAG || E820_Table[i].type != MULTIBOOT_MEMORY_AVAILABLE) continue;
		if (E820_Table[i].base + E820_Table[i].size < mem_max) continue;
		mem_max = E820_Table[i].base + E820_Table[i].size;
	}
	return mem_max;
}

bool RAM_IsMemoryPresent(uintptr_t min, uintptr_t max, struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size) {

	if (min >= max) return false;
	if (E820_Table_size == 0 || E820_Table == MEMORY_NULL_PTR) return false;
	
	uintptr_t mem_max = RAM_MaxPresentMemoryAddress(E820_Table, E820_Table_size);
	if (min >= mem_max || max > mem_max) return false;
	
	for (size_t i = 0; i < E820_Table_size; i++) {
		if (E820_Table[i].acpi3 != MULTIBOOT_MEMORY_ACPI3_FLAG || E820_Table[i].type != MULTIBOOT_MEMORY_AVAILABLE) continue;
		if (E820_Table[i].base <= min && E820_Table[i].base + E820_Table[i].size >= max) return true;
	}
	
	for (size_t i = 0; i < E820_Table_size; i++) {
		if (E820_Table[i].acpi3 != MULTIBOOT_MEMORY_ACPI3_FLAG || E820_Table[i].type != MULTIBOOT_MEMORY_AVAILABLE) continue;
		uint8_t fit = 0;
		if (E820_Table[i].base <= min && E820_Table[i].base + E820_Table[i].size > min) fit += 1;
		if (E820_Table[i].base + E820_Table[i].size >= max && E820_Table[i].base < max) fit += 2;
		
		if      (fit == 0) continue;
		else if (fit == 1) return RAM_IsMemoryPresent(E820_Table[i].base + E820_Table[i].size, max, E820_Table, E820_Table_size);
		else if (fit == 2) return RAM_IsMemoryPresent(min, E820_Table[i].base, E820_Table, E820_Table_size);
		else return true;
	}
	return false;
}

uintptr_t RAM_StoreInfo(uintptr_t addr, struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size) {
	size_t table_size  = 0;
	struct Info_Entry* table_ptr = (struct Info_Entry*)(addr);
	table_ptr->address = 0;
	table_ptr->size = 0;
	
	uintptr_t mem = 0;
	uintptr_t mem_end = RAM_MaxPresentMemoryAddress(E820_Table, E820_Table_size);
	while (mem < mem_end) {
		mem += MEMORY_SIZE_PAGE;
		if (mem < mem-MEMORY_SIZE_PAGE || mem > mem_end) break;
		
		if (RAM_IsMemoryPresent(mem-MEMORY_SIZE_PAGE, mem, E820_Table, E820_Table_size)) {
			if (table_ptr->size == 0) table_size++;
			table_ptr->size += MEMORY_SIZE_PAGE;
		}
		else {
			if (table_ptr->size == 0) table_ptr->address = mem;
			else {
				table_ptr++;
				table_ptr->address = mem;
				table_ptr->size = 0;
			}
		}
	}
	table_size *= sizeof(struct Info_Entry);

	return addr + table_size;
}
