#include <boot/include/memory.h>
#include <boot/include/bios.h>
#include <boot/include/multiboot.h>
#include <csupport/include/string.h>

uint32_t Memory_StoreBasicInfo(uint32_t addr) {

	uint32_t* mem = (uint32_t*)(addr);
	
	// INT 0x12 is the most reliable way to get the size of low memory; but it does not give a map of usable areas
	// AX contains the memory size in KB
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

uint32_t Memory_StoreE820Info(uint32_t addr) {

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

uint32_t Memory_StoreInfo(uint32_t addr, bool page_align, struct Multiboot_E820_Entry* E820_Table, uint32_t E820_Table_size) {
	uint32_t table_size  = 0;
	struct Boot_Block64* table_ptr = (struct Boot_Block64*)(addr);
	table_ptr->address = 0;
	table_ptr->size = 0;
	
	// Create a map of free memory from the E820 entries 
	uint64_t addr_limit = ~((uint64_t)0);
	while (true) {
		uint64_t closest_boundary = addr_limit;
		for (uint32_t i = 0; i < E820_Table_size; i++) {
			if (E820_Table[i].acpi3 != MEMORY_E820_ACPI3_FLAG || E820_Table[i].type != MEMORY_E820_AVAILABLE) continue;
			if (E820_Table[i].base >= table_ptr->address && E820_Table[i].base < closest_boundary) closest_boundary = E820_Table[i].base;
		}
		if (closest_boundary == addr_limit) break;
		
		table_ptr->address = closest_boundary;
		table_ptr->size = 0;
		for (uint32_t i = 0; i < E820_Table_size; i++) {
			if (E820_Table[i].acpi3 != MEMORY_E820_ACPI3_FLAG || E820_Table[i].type != MEMORY_E820_AVAILABLE) continue;
			uint64_t e820_entry_end  = E820_Table[i].base + E820_Table[i].size;
			uint64_t table_entry_end = table_ptr->address + table_ptr->size;
			if (e820_entry_end >= table_entry_end && E820_Table[i].base <= table_entry_end) table_ptr->size = e820_entry_end - table_ptr->address;
		}
		table_ptr++;
		table_size++;
		table_ptr->address = (table_ptr-1)->address + (table_ptr-1)->size;
	}
	table_ptr = (struct Boot_Block64*)(addr);
	
	// Adjust the entries on 4KB boundaries
	if (page_align) {
		for (uint32_t i = 0; i < table_size; i++) {
			uint64_t mask = ~((uint64_t)(0xFFF));
			uint64_t min_addr = table_ptr[i].address & mask;
			uint64_t max_addr = (table_ptr[i].address + table_ptr[i].size) & mask;
			if (min_addr < table_ptr[i].address) min_addr += 0x1000;
			table_ptr[i].address = max_addr > min_addr ? min_addr : 0;
			table_ptr[i].size    = max_addr > min_addr ? max_addr - min_addr : 0;
		}
	}
	
	// Remove 0-sized entries
	for (uint32_t i = 0; i < table_size; i++) {
		if (table_ptr[i].size == 0) {
			memmove(table_ptr+i, table_ptr+i+1, (table_size-i-1)*sizeof(struct Boot_Block64));
			table_size--;
		}
	}
	
	return addr + table_size * sizeof(struct Boot_Block64);
}

// Function to find a contiguous chunk of available memory either above or below a given address
// The starting address of the chunk (which is returned by the function) may need to conform to a certain alignment if the 'align' paremeter is greater than 1
uint32_t Memory_FindBlockAddress(uint32_t addr, bool above, uint32_t size, uint32_t align, struct Boot_Block64* mmap, uint32_t mmap_size) {

	if (size == 0) return MEMORY_32BIT_LIMIT;
    if ( above && (addr == MEMORY_32BIT_LIMIT || addr + size < addr) ) return MEMORY_32BIT_LIMIT;
	if (!above && (addr == 0 || addr < size) )return MEMORY_32BIT_LIMIT;
   
    uint32_t mem = (above ? MEMORY_32BIT_LIMIT : 0);
	uint32_t a = (above ? addr : addr - size); 
    uint32_t aligned_addr = ( (align <= 1 || a % align == 0) ? a : align * ((above ? 1: 0) + a/align) );

    if (mmap_size == 0 || mmap == MEMORY_NULL_PTR) return mem;
    for (uint32_t i = 0; i < mmap_size; i++) {
		if (mmap[i].address >= MEMORY_32BIT_LIMIT) continue;
		
		uint32_t mmap_entry_base = (uint32_t)mmap[i].address;
		uint32_t mmap_entry_size = (mmap[i].address + mmap[i].size > MEMORY_32BIT_LIMIT ? MEMORY_32BIT_LIMIT - mmap[i].address : (uint32_t)mmap[i].size);
		if (mmap_entry_size < size) continue;
		
		uint32_t shifted_base = mmap_entry_base;
		if (!above) {
			if (mmap_entry_base + mmap_entry_size - size >= aligned_addr) shifted_base = aligned_addr;
			else shifted_base = mmap_entry_base + mmap_entry_size - size;  
		}
		uint32_t aligned_base = ( (align <= 1 || shifted_base % align == 0) ? shifted_base : align * ((above ? 1: 0) + shifted_base/align) );
		if (aligned_base < mmap_entry_base || aligned_base >= mmap_entry_base + mmap_entry_size) continue;
		uint64_t aligned_size = mmap_entry_base + mmap_entry_size - aligned_base;
		
		if (mmap_entry_base <= aligned_addr && mmap_entry_base + mmap_entry_size > aligned_addr + size) return aligned_addr;
		if ( above && aligned_size >= size && aligned_base >= addr && aligned_base < mem) mem = aligned_base;
		if (!above && aligned_size >= size && aligned_base <  addr && aligned_base > mem) mem = aligned_base;
    }

    if (above || mem != 0) return mem;
    else return MEMORY_32BIT_LIMIT;
}

