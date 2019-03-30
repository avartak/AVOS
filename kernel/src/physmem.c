#include <kernel/include/physmem.h>
#include <kernel/include/paging.h>

struct Memory_Map Memory_Physical_high;
struct Memory_Map Memory_Physical_dma;

uint8_t   Dispensary_pagemap[DISPENSARY_SIZE];
uint32_t  Dispensary_pagetable[0x400]__attribute__((aligned(0x1000)));
struct    Memory_NodeDispenser* Dispensary_nodepot = MEMORY_NULL_PTR;

extern uint32_t RAM_Table_size;
extern struct   Memory_RAM_Table_Entry RAM_Table;

bool Memory_Physical_AllocateBlock(uintptr_t virtual_address) {
	struct Memory_Node* mem_node = Memory_Map_Pop(&Memory_Physical_high);
	if (mem_node == MEMORY_NULL_PTR) return false;

	if (!Paging_TableExists(virtual_address)) {
		struct Memory_Node* table_node = Memory_Map_Pop(&Memory_Physical_high);
		if (table_node == MEMORY_NULL_PTR) {
			Memory_Map_Push(&Memory_Physical_high, mem_node, true);
			return false;
		}
		Paging_MapEntry(Paging_directory, table_node->pointer, Paging_GetDirectoryEntry(virtual_address), PAGING_KERN_TABLE_FLAGS);
		Paging_ClearTable(virtual_address);
		Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_directory));
	}
	
	if (!(Paging_MapVirtualPage(virtual_address, mem_node->pointer, PAGING_KERN_PAGE_FLAGS))) {
		Memory_Map_Push(&Memory_Physical_high, mem_node, true);
		return false;
	}

	return true;
}

bool Memory_Physical_FreeBlock(uintptr_t virtual_address) {
	struct Memory_Node* node = Memory_NodeDispenser_Dispense(Memory_Physical_high.node_dispenser);
	uintptr_t physical_address = Paging_GetPhysicalAddress(virtual_address);

	if (((uintptr_t)node & MEMORY_PAGE_MASK) == (virtual_address & MEMORY_PAGE_MASK) || !(Paging_UnmapVirtualPage(virtual_address))) {
		Memory_NodeDispenser_Return(node);
		return false;
	}

	node->pointer = physical_address;
	node->size    = 1;
	node->attrib  = Memory_Physical_high.attrib;
	node->next    = MEMORY_NULL_PTR;
	
	return Memory_Map_Push(&Memory_Physical_high, node, true);
}

void Memory_Physical_MakeMap(struct Memory_Map* mem_map, uintptr_t mem_start, uintptr_t mem_end) {
    struct Memory_RAM_Table_Entry* table_ptr = &RAM_Table;
   
    if (mem_start < table_ptr[0].pointer) return;
   
    for (size_t i = 0; i < RAM_Table_size && table_ptr[i].pointer < mem_end; i++) {
        struct Memory_Node* node = Memory_NodeDispenser_Dispense(mem_map->node_dispenser);
        if (node == MEMORY_NULL_PTR) return;
        node->pointer = table_ptr[i].pointer;
        node->size    = (mem_end > table_ptr[i].pointer + table_ptr[i].size ? table_ptr[i].size : mem_end - table_ptr[i].pointer) / MEMORY_SIZE_PAGE;
        node->attrib  = mem_map->attrib;
        node->next    = MEMORY_NULL_PTR;
        Memory_Map_Insert(mem_map, node, true);
    }
}

void Memory_Physical_Initialize() {

	// Lets create the virtual space for the heap
	Paging_MapEntry(Paging_directory, Paging_GetPhysicalAddress((uintptr_t)Dispensary_pagetable), Paging_GetDirectoryEntry(MEMORY_START_DISP), PAGING_KERN_TABLE_FLAGS);
	Paging_ClearTable(MEMORY_START_DISP);
	Paging_MapVirtualPage(MEMORY_START_DISP, MEMORY_PHYSICAL_START_HIGHMEM, PAGING_KERN_PAGE_FLAGS);
	Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_directory));

	// Lets setup the node dispenser at the very start of the heap
	Dispensary_nodepot = (struct Memory_NodeDispenser*)MEMORY_START_DISP;	
    Dispensary_nodepot->freenode = DISPENSER_FIRST_NODE(Dispensary_nodepot);
    Dispensary_nodepot->size = DISPENSER_FULL_SIZE;
    Dispensary_nodepot->attrib = 0;
    Dispensary_nodepot->next = MEMORY_NULL_PTR;
    struct Memory_Node* nodes = (struct Memory_Node*)DISPENSER_FIRST_NODE(Dispensary_nodepot);
    for (size_t i = 0; i < Dispensary_nodepot->size; i++) nodes[i].attrib = Memory_Physical_high.attrib | 0xFF;

	// Here we initialize the bitmap
	for (size_t i = 0; i < DISPENSARY_SIZE; i++) Dispensary_pagemap[i] = 0;
	Dispensary_pagemap[0] = 1;

	// Here we set up the map for the free high memory
	struct Memory_Node* free_node = Memory_NodeDispenser_Dispense(Dispensary_nodepot);
	Memory_Physical_high.start  = free_node;
	Memory_Physical_high.size   = 0x400000/MEMORY_SIZE_PAGE - 1;
	Memory_Physical_high.attrib = MEMORY_4KB << 8;
    Memory_Physical_high.node_dispenser = Dispensary_nodepot;

	free_node->attrib  = Memory_Physical_high.attrib;
	free_node->pointer = MEMORY_PHYSICAL_START_HIGHMEM + MEMORY_SIZE_PAGE;
	free_node->size    = Memory_Physical_high.size;
	free_node->next    = MEMORY_NULL_PTR; 

	Memory_Physical_MakeMap(&Memory_Physical_high, MEMORY_PHYSICAL_START_HIGHMEM + 0x400000, MEMORY_MAX_ADDRESS);

	// Here we set up the map for the free low memory
    Memory_Physical_dma.start  = MEMORY_NULL_PTR;
    Memory_Physical_dma.size   = 0;
    Memory_Physical_dma.attrib = MEMORY_4KB << 8;
    Memory_Physical_dma.node_dispenser = Dispensary_nodepot;

	Memory_Physical_MakeMap(&Memory_Physical_dma, MEMORY_PHYSICAL_START_DMA, MEMORY_PHYSICAL_START_HIGHMEM);
}


