#include <x86/boot/include/ram.h>
#include <x86/boot/include/e820.h>

extern struct Info_Entry BootInfo_Table;

uintptr_t RAM_MaxPresentMemoryAddress() {
	struct Info_Entry* BootInfo_Ptr = &BootInfo_Table;
	struct E820_Table_Entry* E820_Table = (struct E820_Table_Entry*)BootInfo_Ptr[BOOTINFO_ENTRY_E820].address;
	size_t E820_Table_size = BootInfo_Ptr[BOOTINFO_ENTRY_E820].size;

	uintptr_t mem_max = 0;
	if (E820_Table_size == 0 || E820_Table == MEMORY_NULL_PTR) return mem_max;
	for (size_t i = 0; i < E820_Table_size; i++) {
		if (E820_Table[i].acpi3 != 1 || E820_Table[i].type != 1) continue;
		if (E820_Table[i].base + E820_Table[i].size < mem_max) continue;
		mem_max = E820_Table[i].base + E820_Table[i].size;
	}
	return mem_max;
}

bool RAM_IsMemoryPresent(uintptr_t min, uintptr_t max) {
	struct Info_Entry* BootInfo_Ptr = &BootInfo_Table;
	struct E820_Table_Entry* E820_Table = (struct E820_Table_Entry*)BootInfo_Ptr[BOOTINFO_ENTRY_E820].address;
	size_t E820_Table_size = BootInfo_Ptr[BOOTINFO_ENTRY_E820].size;

	if (min >= max) return false;
	if (E820_Table_size == 0 || E820_Table == MEMORY_NULL_PTR) return false;
	
	uintptr_t mem_max = RAM_MaxPresentMemoryAddress();
	if (min >= mem_max || max > mem_max) return false;
	
	for (size_t i = 0; i < E820_Table_size; i++) {
		if (E820_Table[i].acpi3 != 1 || E820_Table[i].type != 1) continue;
		if (E820_Table[i].base <= min && E820_Table[i].base + E820_Table[i].size >= max) return true;
	}
	
	for (size_t i = 0; i < E820_Table_size; i++) {
		if (E820_Table[i].acpi3 != 1 || E820_Table[i].type != 1) continue;
		uint8_t fit = 0;
		if (E820_Table[i].base <= min && E820_Table[i].base + E820_Table[i].size > min) fit += 1;
		if (E820_Table[i].base + E820_Table[i].size >= max && E820_Table[i].base < max) fit += 2;
		
		if      (fit == 0) continue;
		else if (fit == 1) return RAM_IsMemoryPresent(E820_Table[i].base + E820_Table[i].size, max);
		else if (fit == 2) return RAM_IsMemoryPresent(min, E820_Table[i].base);
		else return true;
	}
	return false;
}

uintptr_t RAM_StoreInfo(uintptr_t addr) {
	size_t table_size  = 0;
	struct Info_Entry* table_ptr = (struct Info_Entry*)(addr);
	table_ptr->address = 0;
	table_ptr->size = 0;
	
	uintptr_t mem = 0;
	uintptr_t mem_end = RAM_MaxPresentMemoryAddress();
	while (mem < mem_end) {
		mem += MEMORY_SIZE_PAGE;
		if (mem < mem-MEMORY_SIZE_PAGE || mem > mem_end) break;
		
		if (RAM_IsMemoryPresent(mem-MEMORY_SIZE_PAGE, mem)) {
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

	if (!RAM_IsMemoryPresent(0x00100000, 0x01000000)) return 0;

	return addr + table_size;
}
