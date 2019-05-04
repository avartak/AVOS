#include <x86/boot/include/ram.h>
#include <x86/boot/include/e820.h>
#include <x86/boot/include/multiboot.h>

extern struct Multiboot_Info_Start Multiboot_Information_start;

uintptr_t RAM_MaxPresentMemoryAddress() {

	struct E820_Table_Entry* E820_Table = MEMORY_NULL_PTR;
	size_t E820_Table_size = 0;

	struct Multiboot_Info_Tag* mbi_tag;
	uintptr_t mbi_addr = (uintptr_t)(&Multiboot_Information_start);
	for (mbi_tag = (struct Multiboot_Info_Tag*)(mbi_addr + 8); mbi_tag->type != 0; mbi_tag = (struct Multiboot_Info_Tag*)((uint8_t*)mbi_tag + ((mbi_tag->size + 7) & ~7))) {
		if (mbi_tag->type != MULTIBOOT_TAG_TYPE_MMAP) continue;
		E820_Table_size = (mbi_tag->size - 16)/sizeof(struct E820_Table_Entry);
		E820_Table = (struct E820_Table_Entry*)(16 + (uintptr_t)mbi_tag);
	}

	uintptr_t mem_max = 0;
	if (E820_Table_size == 0 || E820_Table == MEMORY_NULL_PTR) return mem_max;
	for (size_t i = 0; i < E820_Table_size; i++) {
		if (E820_Table[i].acpi3 != MULTIBOOT_MEMORY_ACPI3_FLAG || E820_Table[i].type != MULTIBOOT_MEMORY_AVAILABLE) continue;
		if (E820_Table[i].base + E820_Table[i].size < mem_max) continue;
		mem_max = E820_Table[i].base + E820_Table[i].size;
	}
	return mem_max;
}

bool RAM_IsMemoryPresent(uintptr_t min, uintptr_t max) {

    struct E820_Table_Entry* E820_Table = MEMORY_NULL_PTR;
    size_t E820_Table_size = 0;

    struct Multiboot_Info_Tag* mbi_tag;
    uintptr_t mbi_addr = (uintptr_t)(&Multiboot_Information_start);
    for (mbi_tag = (struct Multiboot_Info_Tag*)(mbi_addr + 8); mbi_tag->type != 0; mbi_tag = (struct Multiboot_Info_Tag*)((uint8_t*)mbi_tag + ((mbi_tag->size + 7) & ~7))) {
        if (mbi_tag->type != MULTIBOOT_TAG_TYPE_MMAP) continue;
        E820_Table_size = (mbi_tag->size - 16)/sizeof(struct E820_Table_Entry);
        E820_Table = (struct E820_Table_Entry*)(16 + (uintptr_t)mbi_tag);
    }

	if (min >= max) return false;
	if (E820_Table_size == 0 || E820_Table == MEMORY_NULL_PTR) return false;
	
	uintptr_t mem_max = RAM_MaxPresentMemoryAddress();
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

	return addr + table_size;
}
