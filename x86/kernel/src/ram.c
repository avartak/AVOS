#include <x86/kernel/include/ram.h>
#include <kernel/include/multiboot.h>

size_t E820_Table_size = 0;
struct E820_Table_Entry* E820_Table = MEMORY_NULL_PTR;

uintptr_t RAM_MaxPresentMemoryAddress() {
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

bool RAM_Initialize() {
	struct Multiboot_Tag* tag;
	for (tag = (struct Multiboot_Tag*) (Multiboot_Info + 2); tag->type != MULTIBOOT2_TAG_TYPE_END; tag = (struct Multiboot_Tag*) ((uint8_t*)tag + ((tag->size + 7) & ~7))) {
		if (tag->type != MULTIBOOT2_TAG_TYPE_MMAP) continue;
		E820_Table_size  = (tag->size - 0x10) / 0x18;
		E820_Table = (struct E820_Table_Entry*)((uint8_t*)tag + 0x10);
	}
	
	if (!RAM_IsMemoryPresent(0x100000, 0x1000000)) return false;

	RAM_Table_size = 0;
	struct Memory_RAM_Table_Entry* table_ptr = &RAM_Table;
	table_ptr->pointer = 0;
	table_ptr->size = 0;
	
	uintptr_t mem = 0;
	uintptr_t mem_end = RAM_MaxPresentMemoryAddress();
	while (mem < mem_end) {
		mem += MEMORY_SIZE_PAGE;
		if (mem < mem-MEMORY_SIZE_PAGE || mem > mem_end) break;
		
		if (RAM_IsMemoryPresent(mem-MEMORY_SIZE_PAGE, mem)) {
			if (table_ptr->size == 0) RAM_Table_size++;
			table_ptr->size += MEMORY_SIZE_PAGE;
		}
		else {
			if (table_ptr->size == 0) table_ptr->pointer = mem;
			else {
				table_ptr++;
				table_ptr->pointer = mem;
				table_ptr->size = 0;
			}
		}
	}

	return true;
}

