#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/paging.h>
#include <kernel/include/multiboot.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

size_t   E820_Table_size = 0;
struct   E820_Table_Entry* E820_Table = MEMORY_NULL_PTR;

struct   Memory_Stack Physical_Memory_usable;
struct   Memory_Stack Physical_Memory_high;
struct   Memory_Stack Physical_Memory_dma;

uint8_t  Kernel_dispensary_map[SIZE_DISPENSARY];
uint32_t Kernel_dispensary_table[0x400]__attribute__((aligned(0x1000)));
struct   Memory_NodeDispenser* Kernel_node_dispenser = MEMORY_NULL_PTR;

bool Physical_Memory_AllocatePage(uintptr_t virtual_address) {
	struct Memory_Node* mem_node = Memory_Stack_Pop(&Physical_Memory_high);
	if (mem_node == MEMORY_NULL_PTR) return false;

	if (!Paging_TableExists(virtual_address)) {
		struct Memory_Node* table_node = Memory_Stack_Pop(&Physical_Memory_high);
		if (table_node == MEMORY_NULL_PTR) {
			Memory_Stack_Push(&Physical_Memory_high, mem_node, true);
			return false;
		}
		Paging_MapTableInDirectory(Paging_directory, table_node->pointer, Paging_GetDirectoryEntry(virtual_address), PAGING_KERN_TABLE_FLAGS);
		Paging_ClearTable(virtual_address);
		Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_directory));
	}
	
	if (!(Paging_MapVirtualToPhysicalPage(virtual_address, mem_node->pointer, PAGING_KERN_PAGE_FLAGS))) {
		Memory_Stack_Push(&Physical_Memory_high, mem_node, true);
		return false;
	}

	return true;
}

bool Physical_Memory_FreePage(uintptr_t virtual_address) {
	struct Memory_Node* node = Memory_NodeDispenser_Dispense(Physical_Memory_high.node_dispenser);
	uintptr_t physical_address = Paging_GetPhysicalAddress(virtual_address);

	if (((uintptr_t)node & PAGE_MASK) == (virtual_address & PAGE_MASK) || !(Paging_UnmapVirtualPage(virtual_address))) {
		Memory_NodeDispenser_Return(node);
		return false;
	}

	node->pointer = physical_address;
	node->size    = 1;
	node->attrib  = Physical_Memory_high.attrib;
	node->next    = MEMORY_NULL_PTR;
	
	return Memory_Stack_Push(&Physical_Memory_high, node, true);
}

uintptr_t Physical_Memory_MaxFreeMemoryAddress(struct E820_Table_Entry* table, size_t size) {
	uintptr_t mem_max = 0;
	if (table == MEMORY_NULL_PTR) return mem_max;
	for (size_t i = 0; i < size; i++) {
		if (table[i].acpi3 != 1 || table[i].type != 1) continue;
		if (table[i].base + table[i].size < mem_max) continue;
		if (mem_max < table[i].base + table[i].size) mem_max = table[i].base + table[i].size;
	}
	return mem_max;
}

bool Physical_Memory_IsRangeFree(uintptr_t min, uintptr_t max, struct E820_Table_Entry* table, size_t size) {
	if (min >= max) return false;
	uintptr_t max_mem = Physical_Memory_MaxFreeMemoryAddress(table, size);
    if (max_mem == 0) return false;
	if (min >= max_mem || max > max_mem) return false;

    for (size_t i = 0; i < size; i++) {
        if (table[i].acpi3 != 1 || table[i].type != 1) continue;
        if (table[i].base <= min && table[i].base + table[i].size >= max) return true;
    }

    for (size_t i = 0; i < size; i++) {
		if (table[i].acpi3 != 1 || table[i].type != 1) continue;
        uint8_t fit = 0;
        if (table[i].base <= min && table[i].base + table[i].size > min) fit += 1;
        if (table[i].base + table[i].size >= max && table[i].base < max) fit += 2;

        if (fit == 0) continue;
        else if (fit == 1) {
			return Physical_Memory_IsRangeFree(table[i].base + table[i].size, max, table, size);
		}
        else if (fit == 2) {
			return Physical_Memory_IsRangeFree(min, table[i].base, table, size);
		}
		else return true;
    }
	return false;
}

void Physical_Memory_MakeMap(struct Memory_Stack* mem_map, uintptr_t map_start, uintptr_t map_end, struct E820_Table_Entry* table_e820, size_t size_e820) {
    uintptr_t scan_point = map_start;
    uintptr_t scan_step0 = MEMORY_SIZE_PAGE * 0x400;
    uintptr_t scan_step1 = MEMORY_SIZE_PAGE;

    while (scan_point < map_end) {
        if (scan_point + scan_step0 <= map_end && Physical_Memory_IsRangeFree(scan_point, scan_point + scan_step0, table_e820, size_e820)) {
            struct Memory_Node* node = Memory_NodeDispenser_Dispense(mem_map->node_dispenser);
            if (node == MEMORY_NULL_PTR) return;
            node->pointer = scan_point;
            node->size = 0x400;
            node->attrib = mem_map->attrib;
            node->next = MEMORY_NULL_PTR;
            Memory_Stack_Insert(mem_map, node, true);
            scan_point += scan_step0;
        }
        else {
            for (size_t i = 0; i < 0x400; i++) {
				if (scan_point + scan_step1 < scan_point || scan_point + scan_step1 > map_end) {
					scan_point = map_end;
					break;
				}
                if (Physical_Memory_IsRangeFree(scan_point, scan_point + scan_step1, table_e820, size_e820)) {
                    struct Memory_Node* node = Memory_NodeDispenser_Dispense(mem_map->node_dispenser);
                    if (node == MEMORY_NULL_PTR) return;
                    node->pointer = scan_point;
                    node->size = 1;
                    node->attrib = mem_map->attrib;
                    node->next = MEMORY_NULL_PTR;
                    Memory_Stack_Insert(mem_map, node, true);
                }
                scan_point += scan_step1;
            }
        }
	}
}


void Physical_Memory_Initialize(uint32_t* mbi) {
	// We point the E820 structure to the memory map produced by the boot loader
    struct Multiboot_Tag* tag;
    for (tag = (struct Multiboot_Tag*) (mbi + 2); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct Multiboot_Tag*) ((uint8_t*)tag + ((tag->size + 7) & ~7))) {
        if (tag->type != MULTIBOOT_TAG_TYPE_MMAP) continue;
        E820_Table_size  = (tag->size - 0x10) / 0x18;
        E820_Table = (struct E820_Table_Entry*)((uint8_t*)tag + 0x10);
    }

	// Lets create the virtual space for the heap
	Paging_MapTableInDirectory(Paging_directory, Paging_GetPhysicalAddress((uintptr_t)Kernel_dispensary_table), Paging_GetDirectoryEntry(VIRTUAL_MEMORY_START_DISP), PAGING_KERN_TABLE_FLAGS);
	Paging_ClearTable(VIRTUAL_MEMORY_START_DISP);
	Paging_MapVirtualToPhysicalPage(VIRTUAL_MEMORY_START_DISP, PHYSICAL_MEMORY_START_HIGHMEM, PAGING_KERN_PAGE_FLAGS);
	Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_directory));

	// Lets setup the node dispenser at the very start of the heap
	Kernel_node_dispenser = (struct Memory_NodeDispenser*)VIRTUAL_MEMORY_START_DISP;	
    Kernel_node_dispenser->freenode = FIRST_NODE(Kernel_node_dispenser);
    Kernel_node_dispenser->size = FULL_DISPENSER_SIZE;
    Kernel_node_dispenser->attrib = 0;
    Kernel_node_dispenser->next = MEMORY_NULL_PTR;
    struct Memory_Node* nodes = (struct Memory_Node*)FIRST_NODE(Kernel_node_dispenser);
    for (size_t i = 0; i < Kernel_node_dispenser->size; i++) nodes[i].attrib = Physical_Memory_high.attrib | 0xFF;

	// Here we initialize the bitmap
	for (size_t i = 0; i < SIZE_DISPENSARY; i++) Kernel_dispensary_map[i] = 0;
	Kernel_dispensary_map[0] = 1;

	// Here we set up the map for the free high memory
	struct Memory_Node* free_node = Memory_NodeDispenser_Dispense(Kernel_node_dispenser);
	Physical_Memory_high.start  = free_node;
	Physical_Memory_high.size   = 0x400000/MEMORY_SIZE_PAGE - 1;
	Physical_Memory_high.attrib = MEMORY_4KB << 8;
    Physical_Memory_high.node_dispenser = Kernel_node_dispenser;

	free_node->attrib  = Physical_Memory_high.attrib;
	free_node->pointer = PHYSICAL_MEMORY_START_HIGHMEM + MEMORY_SIZE_PAGE;
	free_node->size    = Physical_Memory_high.size;
	free_node->next    = MEMORY_NULL_PTR; 

	Physical_Memory_MakeMap(&Physical_Memory_high, PHYSICAL_MEMORY_START_HIGHMEM + 0x400000, Physical_Memory_MaxFreeMemoryAddress(E820_Table, E820_Table_size), E820_Table, E820_Table_size);

	// Here we set up the map for the free low memory
    Physical_Memory_dma.start  = MEMORY_NULL_PTR;
    Physical_Memory_dma.size   = 0;
    Physical_Memory_dma.attrib = MEMORY_4KB << 8;
    Physical_Memory_dma.node_dispenser = Kernel_node_dispenser;

	Physical_Memory_MakeMap(&Physical_Memory_dma, PHYSICAL_MEMORY_START_DMA, PHYSICAL_MEMORY_START_HIGHMEM, E820_Table, E820_Table_size);

	// Here we set up the permanent/static map of usable RAM
    Physical_Memory_usable.start  = MEMORY_NULL_PTR;
    Physical_Memory_usable.size   = 0;
    Physical_Memory_usable.attrib = MEMORY_4KB << 8;
    Physical_Memory_usable.node_dispenser = Kernel_node_dispenser;

	Physical_Memory_MakeMap(&Physical_Memory_usable, 0, 0xFFFFFFFF, E820_Table, E820_Table_size);
}


