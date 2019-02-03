#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/paging.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

uint32_t E820_Table_size;
struct   E820_Table_Entry* E820_Table;
struct   Memory_FIFO Physical_Memory_free;
struct   Memory_FIFO Physical_Memory_dma;
struct   Memory_FIFO kernel_heap;

bool Physical_Memory_AllocatePage(uintptr_t virtual_address) {
	struct Memory_Node* mem_node = Memory_Pop(&Physical_Memory_free);
	if (mem_node == MEMORY_NULL_PTR) return false;

	if (!Paging_TableExists(virtual_address)) {
		struct Memory_Node* table_node = Memory_Pop(&Physical_Memory_free);
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
	return Memory_Push(&Physical_Memory_free, node);
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

bool Physical_Memory_IsPageFree(uintptr_t page, struct E820_Table_Entry* table, uint32_t size) {
	uint32_t page_min = page & 0xFFFFF000;
	uint32_t page_max = page | 0x00000FFF;

	return Physical_Memory_IsRangeFree(page_min, page_max, table, size);
}

void Physical_Memory_Initialize() {
	Paging_MapTableInDirectory(Paging_kernel_directory, Paging_GetPhysicalAddress((uintptr_t)Paging_kernel_heaptable), Paging_GetDirectoryEntry(VIRTUAL_MEMORY_START_HEAP), PAGING_KERN_TABLE_FLAGS);
	Paging_ClearTable(VIRTUAL_MEMORY_START_HEAP);
	Paging_MapVirtualToPhysicalPage(VIRTUAL_MEMORY_START_HEAP, PHYSICAL_MEMORY_START_HEAP, PAGING_KERN_PAGE_FLAGS);
	Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_kernel_directory));

	E820_Table_size = *((uint32_t*)VIRTUAL_MEMORY_E820_TABLE_PTR);
	E820_Table = (struct E820_Table_Entry*)(VIRTUAL_MEMORY_E820_TABLE_PTR+4);

	// Here we initialize the heap map
	kernel_heap.start = MEMORY_NULL_PTR;
	kernel_heap.size = 0;
	kernel_heap.attrib = 0x10 << 8;
    kernel_heap.node_dispenser = (struct Memory_NodeDispenser*)VIRTUAL_MEMORY_START_HEAP;
	

	// Here we set up the map for the free high memory
	Physical_Memory_free.start = MEMORY_NULL_PTR;
	Physical_Memory_free.size = 0;
	Physical_Memory_free.attrib = 0x10 << 8;
    Physical_Memory_free.node_dispenser = (struct Memory_NodeDispenser*)VIRTUAL_MEMORY_START_HEAP;

	struct Memory_NodeDispenser* dispenser = Physical_Memory_free.node_dispenser;	
    dispenser->freenode = (uintptr_t)dispenser + sizeof(struct Memory_NodeDispenser);
    dispenser->size = (MEMORY_SIZE_PAGE - sizeof(struct Memory_NodeDispenser)) / sizeof(struct Memory_Node);
    dispenser->attrib = 0;
    dispenser->next = MEMORY_NULL_PTR;
    struct Memory_Node* nodes = (struct Memory_Node*)((uintptr_t)dispenser + sizeof(struct Memory_NodeDispenser));
    for (size_t i = 0; i < dispenser->size; i++) nodes[i].attrib = Physical_Memory_free.attrib | 0x000000FF;

	struct Memory_Node* free_node = Memory_NodeDispenser_Dispense(dispenser);
	Physical_Memory_free.start = free_node;
	free_node->attrib  = Physical_Memory_free.attrib;
	free_node->pointer = PHYSICAL_MEMORY_START_HEAP + MEMORY_SIZE_PAGE;
	free_node->size    = (PHYSICAL_MEMORY_START_DMA - free_node->pointer)/MEMORY_SIZE_PAGE;
	free_node->next    = MEMORY_NULL_PTR;

	// Here we setup the heap map
	free_node = Memory_NodeDispenser_Dispense(dispenser);
	kernel_heap.start  = free_node;
	kernel_heap.size   = (VIRTUAL_MEMORY_END_HEAP - VIRTUAL_MEMORY_START_HEAP)/MEMORY_SIZE_PAGE - 1;
	free_node->attrib  = kernel_heap.attrib;
	free_node->pointer = VIRTUAL_MEMORY_START_HEAP + MEMORY_SIZE_PAGE;
	free_node->size    = kernel_heap.size;
	free_node->next    = MEMORY_NULL_PTR;

	uint32_t scan_point = PHYSICAL_MEMORY_START_HIGHMEM;
	uint32_t scan_step0 = MEMORY_SIZE_PAGE * 0x400;
	uint32_t scan_step1 = MEMORY_SIZE_PAGE;

	while (scan_point < Physical_Memory_MaxFreeMemoryAddress(E820_Table, E820_Table_size)) {
		if (Physical_Memory_IsRangeFree(scan_point, scan_point + scan_step0, E820_Table, E820_Table_size)) {
			free_node = Memory_NodeDispenser_Dispense(dispenser);
			if (free_node == MEMORY_NULL_PTR) return;
			free_node->pointer = scan_point;
			free_node->size = 0x400;
			free_node->attrib = Physical_Memory_free.attrib;
			free_node->next = MEMORY_NULL_PTR;
			Memory_Append(&Physical_Memory_free, free_node);
			scan_point += scan_step0;
		}
		else {
			for (size_t i = 0; i < 0x400; i++) {
				if (Physical_Memory_IsRangeFree(scan_point, scan_point + scan_step1, E820_Table, E820_Table_size)) {
					free_node = Memory_NodeDispenser_Dispense(dispenser);
					if (free_node == MEMORY_NULL_PTR) return;
					free_node->pointer = scan_point;
					free_node->size = 1;
					free_node->attrib = Physical_Memory_free.attrib;
					free_node->next = MEMORY_NULL_PTR;
					Memory_Append(&Physical_Memory_free, free_node);
				}
				scan_point += scan_step1;
			}
		}
	}

	// Here we set up the map for the free low memory
    Physical_Memory_dma.start = MEMORY_NULL_PTR;
    Physical_Memory_dma.size = 0;
    Physical_Memory_dma.attrib = 0x10 << 8;
    Physical_Memory_dma.node_dispenser = dispenser;

    scan_point = PHYSICAL_MEMORY_START_DMA;

	struct Memory_Node* dma_node = MEMORY_NULL_PTR;
    while (scan_point < PHYSICAL_MEMORY_START_HIGHMEM) {
        if (Physical_Memory_IsRangeFree(scan_point, scan_point + scan_step0, E820_Table, E820_Table_size)) {
            dma_node = Memory_NodeDispenser_Dispense(dispenser);
            if (dma_node == MEMORY_NULL_PTR) return;
            dma_node->pointer = scan_point;
            dma_node->size = 0x400;
            dma_node->attrib = Physical_Memory_dma.attrib;
            dma_node->next = MEMORY_NULL_PTR;
            Memory_Append(&Physical_Memory_dma, dma_node);
            scan_point += scan_step0;
        }
        else {
            for (size_t i = 0; i < 0x400; i++) {
                if (Physical_Memory_IsRangeFree(scan_point, scan_point + scan_step1, E820_Table, E820_Table_size)) {
                    dma_node = Memory_NodeDispenser_Dispense(dispenser);
                    if (dma_node == MEMORY_NULL_PTR) return;
                    dma_node->pointer = scan_point;
                    dma_node->size = 1;
                    dma_node->attrib = Physical_Memory_dma.attrib;
                    dma_node->next = MEMORY_NULL_PTR;
                    Memory_Append(&Physical_Memory_dma, dma_node);
                }
                scan_point += scan_step1;
            }
        }
    }
}

