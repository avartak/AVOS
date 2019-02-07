#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/paging.h>
#include <kernel/include/multiboot.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

uint32_t E820_Table_size;
struct   E820_Table_Entry* E820_Table;

struct   Memory_Stack Physical_Memory_free;
struct   Memory_Stack Physical_Memory_dma;
struct   Memory_Stack Virtual_Memory_free;
struct   Memory_Stack Virtual_Memory_inuse;

bool Physical_Memory_AllocatePage(uintptr_t virtual_address) {
	struct Memory_Node* mem_node = Memory_Stack_Pop(&Physical_Memory_free);
	if (mem_node == MEMORY_NULL_PTR) return false;

	if (!Paging_TableExists(virtual_address)) {
		struct Memory_Node* table_node = Memory_Stack_Pop(&Physical_Memory_free);
		if (table_node == MEMORY_NULL_PTR) return false;
		Paging_MapTableInDirectory(Paging_kernel_directory, table_node->pointer, Paging_GetDirectoryEntry(virtual_address), PAGING_KERN_TABLE_FLAGS);
		Paging_ClearTable(virtual_address);
		Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_kernel_directory));
	}
	
	if (!(Paging_MapVirtualToPhysicalPage(virtual_address, mem_node->pointer, PAGING_KERN_PAGE_FLAGS))) return false;
	else return true;
}

bool Physical_Memory_FreePage(uintptr_t virtual_address) {
	struct Memory_Node* node = Memory_NodeDispenser_Dispense(Physical_Memory_free.node_dispenser);
	if (((uintptr_t)node & 0xFFFFF000) == (virtual_address & 0xFFFFF000)) return false;
	node->pointer = Paging_GetPhysicalAddress(virtual_address);
	node->size    = 1;
	node->attrib  = Physical_Memory_free.attrib;
	node->next    = MEMORY_NULL_PTR;
	
	if (!(Paging_UnmapVirtualPage(virtual_address))) return false;
	return Memory_Stack_Push(&Physical_Memory_free, node);
}

uint32_t Physical_Memory_MaxFreeMemoryAddress(struct E820_Table_Entry* table, uint32_t size) {
	uint32_t mem_max = 0;
	if (table == MEMORY_NULL_PTR) return mem_max;
	for (size_t i = 0; i < size; i++) {
		if (table[i].acpi3 != 1 || table[i].type != 1) continue;
		if (table[i].base + table[i].size < mem_max) continue;
		mem_max = table[i].base + table[i].size;
	}
	return mem_max;
}

bool Physical_Memory_IsRangeFree(uintptr_t min, uintptr_t max, struct E820_Table_Entry* table, uint32_t size) {
	if (min >= max) return false;
	uint32_t max_mem = Physical_Memory_MaxFreeMemoryAddress(table, size);
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

void Physical_Memory_MakeMap(struct Memory_Stack* mem_map, uintptr_t map_start, uintptr_t map_end, struct E820_Table_Entry* table_e820, uint32_t size_e820) {
    uint32_t scan_point = map_start;
    uint32_t scan_step0 = MEMORY_SIZE_PAGE * 0x400;
    uint32_t scan_step1 = MEMORY_SIZE_PAGE;

    while (scan_point < map_end) {
        if (Physical_Memory_IsRangeFree(scan_point, scan_point + scan_step0, table_e820, size_e820)) {
            struct Memory_Node* node = Memory_NodeDispenser_Dispense(mem_map->node_dispenser);
            if (node == MEMORY_NULL_PTR) return;
            node->pointer = scan_point;
            node->size = 0x400;
            node->attrib = mem_map->attrib;
            node->next = MEMORY_NULL_PTR;
            Memory_Stack_Append(mem_map, node);
            scan_point += scan_step0;
        }
        else {
            for (size_t i = 0; i < 0x400; i++) {
                if (Physical_Memory_IsRangeFree(scan_point, scan_point + scan_step1, E820_Table, E820_Table_size)) {
                    struct Memory_Node* node = Memory_NodeDispenser_Dispense(mem_map->node_dispenser);
                    if (node == MEMORY_NULL_PTR) return;
                    node->pointer = scan_point;
                    node->size = 1;
                    node->attrib = mem_map->attrib;
                    node->next = MEMORY_NULL_PTR;
                    Memory_Stack_Append(mem_map, node);
                }
                scan_point += scan_step1;
            }
        }
    }
}


void Memory_Initialize(uint32_t* mbi) {
	// We point the E820 structure to the memory map produced by the boot loader
    E820_Table_size = 0;
    struct Multiboot_Tag* tag;
    for (tag = (struct Multiboot_Tag*) (mbi + 2); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct Multiboot_Tag*) ((uint8_t*)tag + ((tag->size + 7) & ~7))) {
        if (tag->type != MULTIBOOT_TAG_TYPE_MMAP) continue;
        E820_Table_size  = (tag->size - 0x10) / 0x18;
        E820_Table = (struct E820_Table_Entry*)((uint8_t*)tag + 0x10);
    }

	// Lets create the virtual space for the heap
	Paging_MapTableInDirectory(Paging_kernel_directory, Paging_GetPhysicalAddress((uintptr_t)Paging_kernel_heaptable), Paging_GetDirectoryEntry(VIRTUAL_MEMORY_START_CHUNK), PAGING_KERN_TABLE_FLAGS);
	Paging_ClearTable(VIRTUAL_MEMORY_START_CHUNK);
	Paging_MapVirtualToPhysicalPage(VIRTUAL_MEMORY_START_CHUNK, PHYSICAL_MEMORY_START_CHUNK, PAGING_KERN_PAGE_FLAGS);
	Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_kernel_directory));

	// Lets setup the node dispenser at the very start of the heap
	struct Memory_NodeDispenser* dispenser = (struct Memory_NodeDispenser*)VIRTUAL_MEMORY_START_CHUNK;	
    dispenser->freenode = (uintptr_t)dispenser + sizeof(struct Memory_NodeDispenser);
    dispenser->size = (MEMORY_SIZE_PAGE - sizeof(struct Memory_NodeDispenser)) / sizeof(struct Memory_Node);
    dispenser->attrib = 0;
    dispenser->next = MEMORY_NULL_PTR;
    struct Memory_Node* nodes = (struct Memory_Node*)((uintptr_t)dispenser + sizeof(struct Memory_NodeDispenser));
    for (size_t i = 0; i < dispenser->size; i++) nodes[i].attrib = Physical_Memory_free.attrib | 0x000000FF;

	struct Memory_Node* free_node;

	// Here we initialize the (free) virtual memory map
	free_node = Memory_NodeDispenser_Dispense(dispenser);
	Virtual_Memory_free.start  = free_node;
	Virtual_Memory_free.size   = (VIRTUAL_MEMORY_END_CHUNK - VIRTUAL_MEMORY_START_CHUNK)/MEMORY_SIZE_PAGE - 1;
	Virtual_Memory_free.attrib = 0x10 << 8;
    Virtual_Memory_free.node_dispenser = dispenser;
	
	free_node->attrib  = Virtual_Memory_free.attrib;
	free_node->pointer = VIRTUAL_MEMORY_START_CHUNK + MEMORY_SIZE_PAGE;
	free_node->size    = Virtual_Memory_free.size;
	free_node->next    = MEMORY_NULL_PTR; 

    // Here we initialize the (in use) virtual memory map
    free_node = Memory_NodeDispenser_Dispense(dispenser);
    Virtual_Memory_inuse.start  = free_node;
    Virtual_Memory_inuse.size   = 1;
    Virtual_Memory_inuse.attrib = 0x10 << 8;
    Virtual_Memory_inuse.node_dispenser = dispenser;
    
    free_node->attrib  = Virtual_Memory_inuse.attrib;
    free_node->pointer = VIRTUAL_MEMORY_START_CHUNK;
    free_node->size    = Virtual_Memory_inuse.size;
    free_node->next    = MEMORY_NULL_PTR; 

	// Here we set up the map for the free high memory
	free_node = Memory_NodeDispenser_Dispense(dispenser);
	Physical_Memory_free.start  = free_node;
	Physical_Memory_free.size   = 0;
	Physical_Memory_free.attrib = 0x10 << 8;
    Physical_Memory_free.node_dispenser = dispenser;

	free_node->attrib  = Physical_Memory_free.attrib;
	free_node->pointer = PHYSICAL_MEMORY_START_CHUNK + MEMORY_SIZE_PAGE;
	free_node->size    = (PHYSICAL_MEMORY_START_DMA - free_node->pointer)/MEMORY_SIZE_PAGE;
	free_node->next    = MEMORY_NULL_PTR; 

	Physical_Memory_MakeMap(&Physical_Memory_free, PHYSICAL_MEMORY_START_HIGHMEM, Physical_Memory_MaxFreeMemoryAddress(E820_Table, E820_Table_size), E820_Table, E820_Table_size);

	// Here we set up the map for the free low memory
    Physical_Memory_dma.start  = MEMORY_NULL_PTR;
    Physical_Memory_dma.size   = 0;
    Physical_Memory_dma.attrib = 0x10 << 8;
    Physical_Memory_dma.node_dispenser = dispenser;

	Physical_Memory_MakeMap(&Physical_Memory_dma, PHYSICAL_MEMORY_START_DMA, PHYSICAL_MEMORY_START_HIGHMEM, E820_Table, E820_Table_size);
}


