#include <x86/kernel/include/checks.h>
#include <kernel/include/multiboot.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool Initial_Checks(uint32_t* mbi) {
	uint32_t size_e820 = 0;
	struct E820_Table_Entry* table_e820;

	struct Multiboot_Tag* tag;

	for (tag = (struct Multiboot_Tag*) (mbi + 2); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct Multiboot_Tag*) ((uint8_t*)tag + ((tag->size + 7) & ~7))) {
		if (tag->type != MULTIBOOT_TAG_TYPE_MMAP) continue;
		size_e820  = (tag->size - 0x10) / 0x18;
		table_e820 = (struct E820_Table_Entry*)((uint8_t*)tag + 0x10);
	}

	if (size_e820 == 0) return false;

	bool check_kernel_code_memory = IsMemoryAvailable( 0x100000,  0x400000, table_e820, size_e820);	
	bool check_kernel_heap_memory = IsMemoryAvailable(0x1000000, 0x1400000, table_e820, size_e820);

	return (check_kernel_code_memory && check_kernel_heap_memory);
}

bool IsMemoryAvailable(uintptr_t min, uintptr_t max, struct E820_Table_Entry* table, uint32_t size) {
    if (min >= max) return false;
    for (size_t i = 0; i < size; i++) {
        if (table[i].type != 1) continue;
        if (table[i].base <= min && table[i].base + table[i].size >= max) return true;
    }
    for (size_t i = 0; i < size; i++) {
        if (table[i].type != 1) continue;
        uint8_t fit = 0;
        if (table[i].base <= min && table[i].base + table[i].size > min) fit += 1;
        if (table[i].base + table[i].size >= max && table[i].base < max) fit += 2;

        if (fit == 0) continue;
        else if (fit == 1) return IsMemoryAvailable(table[i].base + table[i].size, max, table, size);
        else if (fit == 2) return IsMemoryAvailable(min, table[i].base, table, size);
        else return true;
    }
    return false;
}
