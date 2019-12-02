#include <boot/x86/BIOS/include/RAM.h>
#include <boot/x86/BIOS/include/multiboot.h>
#include <boot/x86/BIOS/include/bios.h>

uintptr_t RAM_StoreBasicInfo(uintptr_t addr) {

	uint32_t* mem = (uint32_t*)(addr);
	
	// INT 0x12 is the most reliable way to get the size of low memory; but it does not give a map if usable areas
    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

	BIOS_Interrupt(0x12, &BIOS_regs);
	if ((BIOS_regs.flags & 1) == 1) return addr;
	mem[0] = BIOS_regs.eax & 0xFFFF;
	if (mem[0] == 0) return addr;

	// To get the size of upper memory (above the 1 MB limit placed by the 20-bit real addressing) we first try INT 0x15, AX=0xE801
	// This routine knows about the 1MB memory hole at address 15 MB, but stops at the next hole / reserved area. 
	// Gives the available memory in KB between 1-15 MB in CX, and the available memory in 64 KB above 16 MB in DX
	// If CX contains zero before and after the routine, check AX/BX instead
	mem[1] = 0;
	BIOS_ClearRegistry(&BIOS_regs);
	BIOS_regs.eax = 0xE801;
	BIOS_Interrupt(0x15, &BIOS_regs);
	if ((BIOS_regs.flags & 1) == 0 && (BIOS_regs.eax & 0xFF00) != 0x8600 && (BIOS_regs.eax & 0xFF00) != 0x8000) {
		if ((BIOS_regs.ecx & 0xFFFF) != 0) {
			mem[1] += (BIOS_regs.ecx & 0xFFFF) + (BIOS_regs.edx & 0xFFFF)*0x40;
			return addr + 8;
		}
		else if ((BIOS_regs.eax & 0xFFFF) != 0) {
			mem[1] += (BIOS_regs.eax & 0xFFFF) + (BIOS_regs.ebx & 0xFFFF)*0x40;
			return addr + 8;
		}
	}

	// If INT 0x15, AX=0xE801 didn't work, we try INT 0x15, AX=0x88
	// This function may stop itself at 15 MB
	BIOS_ClearRegistry(&BIOS_regs);
	BIOS_regs.eax = 0x88;
	BIOS_Interrupt(0x15, &BIOS_regs);
	
	if ((BIOS_regs.flags & 1) == 1) return addr;
	mem[1] = (BIOS_regs.eax & 0xFFFF);
	return addr + 8;
}

uintptr_t RAM_StoreE820Info(uintptr_t addr) {

	// INT 0x15, EAX = 0xE820 is the best way to get the detailed memory map of the system
	// Memory map is set up at the designated memory address in ES:DI
	// Each entry in the map is typically 20 bytes, but can be 24-byte long
	// First 8 bytes carry the 64-bit start address of the memory block
	// Next 8 bytes carry the size of the memory block 
	// Next 4 bytes carry the type information about the memory
	// - 1 : Usable or available or free RAM
	// - 2 : Reserved (do not use)
	// - 3 : ACPI reclaimable memory 
	// - 4 : ACPI NVS memory
	// - 5 : Bad memory
	// Last 4 bytes if available are meant for ACPI 3.0 extensions
	// Bit 0 of the last 4 bytes : If clear then ignore the block
	// Bit 1 of the last 4 bytes : If set then the memory in the entry is non-volatile

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

        if ((BIOS_regs.flags & 1) == 1 || BIOS_regs.eax != 0x534D4150 || BIOS_regs.ebx == 0) break;

        if ((BIOS_regs.ecx <= 20 || (current_entry->acpi3 & 3) == 1) && current_entry->size > 0) {
            BIOS_regs.esi += 0x18;
            BIOS_regs.edi += 0x18;
            current_entry++;
        }
    }

    return addr + BIOS_regs.esi;

}


uint64_t RAM_MaxPresentMemoryAddress(struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size) {

	// Find the maximum memory address that exists physically and is available
	// Iterate over the E820 memory map to get this information

	uint64_t mem_max = 0;
	if (E820_Table_size == 0 || E820_Table == MEMORY_NULL_PTR) return mem_max;
	for (size_t i = 0; i < E820_Table_size; i++) {
		if (E820_Table[i].acpi3 != MULTIBOOT_MEMORY_ACPI3_FLAG || E820_Table[i].type != MULTIBOOT_MEMORY_AVAILABLE) continue;
		if (E820_Table[i].base + E820_Table[i].size > mem_max) mem_max = E820_Table[i].base + E820_Table[i].size;
	}
	return mem_max;
}

bool RAM_IsMemoryPresent(uint64_t min, uint64_t max, struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size) {

	if (min >= max) return false;
	if (E820_Table_size == 0 || E820_Table == MEMORY_NULL_PTR) return false;
	
	uint64_t mem_max = RAM_MaxPresentMemoryAddress(E820_Table, E820_Table_size);
	if (min >= mem_max || max > mem_max) return false;

	// First check if the memory range is covered by a single entry 
	for (size_t i = 0; i < E820_Table_size; i++) {
		if (E820_Table[i].acpi3 != MULTIBOOT_MEMORY_ACPI3_FLAG || E820_Table[i].type != MULTIBOOT_MEMORY_AVAILABLE) continue;
		if (E820_Table[i].base <= min && E820_Table[i].base + E820_Table[i].size >= max) return true;
	}
	
	// Next check if the memory range is covered by multiple entries
	for (size_t i = 0; i < E820_Table_size; i++) {
		uint64_t e820_entry_end = E820_Table[i].base + E820_Table[i].size;
		if (E820_Table[i].acpi3 != MULTIBOOT_MEMORY_ACPI3_FLAG || E820_Table[i].type != MULTIBOOT_MEMORY_AVAILABLE) continue;
		if (E820_Table[i].base <= min && e820_entry_end > min) return RAM_IsMemoryPresent(e820_entry_end, max, E820_Table, E820_Table_size);
		if (e820_entry_end >= max && E820_Table[i].base < max) return RAM_IsMemoryPresent(min, E820_Table[i].base, E820_Table, E820_Table_size);
	}
	return false;
}

uintptr_t RAM_StoreInfo(uintptr_t addr, struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size) {
    size_t table_size  = 0;
    struct Boot_Block64_Entry* table_ptr = (struct Boot_Block64_Entry*)(addr);
    table_ptr->address = 0;
    table_ptr->size = 0;

	// Create a map of free memory from the E820 entries 
	uint64_t mem_end = RAM_MaxPresentMemoryAddress(E820_Table, E820_Table_size);
	while (table_ptr->address < mem_end) {
		uint64_t addr_limit = ~((uint64_t)0);
		uint64_t closest_boundary = addr_limit;
		for (size_t i = 0; i < E820_Table_size; i++) {
			if (E820_Table[i].acpi3 != MULTIBOOT_MEMORY_ACPI3_FLAG || E820_Table[i].type != MULTIBOOT_MEMORY_AVAILABLE) continue;
			if (E820_Table[i].base >= table_ptr->address && E820_Table[i].base < closest_boundary) closest_boundary = E820_Table[i].base;
		}
		if (closest_boundary == addr_limit) break;

		table_ptr->address = closest_boundary;
		table_ptr->size = 0;
		for (size_t i = 0; i < E820_Table_size; i++) {
			if (E820_Table[i].acpi3 != MULTIBOOT_MEMORY_ACPI3_FLAG || E820_Table[i].type != MULTIBOOT_MEMORY_AVAILABLE) continue;
			uint64_t e820_entry_end = E820_Table[i].base + E820_Table[i].size;
			if (E820_Table[i].base <= table_ptr->address && e820_entry_end >= table_ptr->address + table_ptr->size) table_ptr->size = e820_entry_end - table_ptr->address;
		}
		table_ptr++;
		table_size++;
		table_ptr->address = (table_ptr-1)->address + (table_ptr-1)->size + 1;
	}

	// Adjust the entries on 4KB boundaries
    table_ptr = (struct Boot_Block64_Entry*)(addr);
	for (size_t i = 0; i < table_size; i++) {
		uint64_t min_addr = table_ptr[i].address;
		uint64_t mask = ~((uint64_t)(0xFFF));
		if ((min_addr & mask) < min_addr) min_addr = (min_addr & mask) + 0x1000;
		uint64_t max_addr = (table_ptr[i].address + table_ptr[i].size) & mask;
		if (max_addr > min_addr) {
			table_ptr[i].address = min_addr;
			table_ptr[i].size = max_addr - min_addr;
		}
		else {
			table_ptr[i].address = 0;
			table_ptr[i].size = 0;
		}
	}

	// Remove entries that are reduced to 0-size after 4KB boundary adjustment
	for (size_t i = 0; i < table_size; i++) {
		if (table_ptr[i].size == 0) {
			for (size_t j = i+i; j < table_size; j++) {
				table_ptr[j-1].address = table_ptr[j].address;
				table_ptr[j-1].size = table_ptr[j].size;
			}
			table_size--;
		}
	}

    return addr + table_size * sizeof(struct Boot_Block64_Entry);
}

