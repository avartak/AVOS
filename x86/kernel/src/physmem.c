#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/paging.h>

struct Memory_Stack Physical_Memory_high;
struct Memory_Stack Physical_Memory_dma;

uint8_t   Kernel_dispensary_map[SIZE_DISPENSARY];
uint32_t  Kernel_dispensary_table[0x400]__attribute__((aligned(0x1000)));
struct    Memory_NodeDispenser* Kernel_node_dispenser = MEMORY_NULL_PTR;

extern uint32_t RAM_Table_size;
extern struct   RAM_Table_Entry RAM_Table;

bool Physical_Memory_AllocatePage(uintptr_t virtual_address) {
	struct Memory_Node* mem_node = Memory_Stack_Pop(&Physical_Memory_high);
	if (mem_node == MEMORY_NULL_PTR) return false;

	if (!Paging_TableExists(virtual_address)) {
		struct Memory_Node* table_node = Memory_Stack_Pop(&Physical_Memory_high);
		if (table_node == MEMORY_NULL_PTR) {
			Memory_Stack_Push(&Physical_Memory_high, mem_node, true);
			return false;
		}
		Paging_MapEntry(Paging_directory, table_node->pointer, Paging_GetDirectoryEntry(virtual_address), PAGING_KERN_TABLE_FLAGS);
		Paging_ClearTable(virtual_address);
		Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_directory));
	}
	
	if (!(Paging_MapVirtualPage(virtual_address, mem_node->pointer, PAGING_KERN_PAGE_FLAGS))) {
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

void Physical_Memory_MakeMap(struct Memory_Stack* mem_map, uintptr_t mem_start, uintptr_t mem_end) {
    struct RAM_Table_Entry* table_ptr = &RAM_Table;
   
    if (mem_start < table_ptr[0].pointer) return;
   
    for (size_t i = 0; i < RAM_Table_size && table_ptr[i].pointer < mem_end; i++) {
        struct Memory_Node* node = Memory_NodeDispenser_Dispense(mem_map->node_dispenser);
        if (node == MEMORY_NULL_PTR) return;
        node->pointer = table_ptr[i].pointer;
        node->size    = (mem_end > table_ptr[i].pointer + table_ptr[i].size ? table_ptr[i].size : mem_end - table_ptr[i].pointer) / MEMORY_SIZE_PAGE;
        node->attrib  = mem_map->attrib;
        node->next    = MEMORY_NULL_PTR;
        Memory_Stack_Insert(mem_map, node, true);
    }
}

void Physical_Memory_Initialize() {
	// Lets create the virtual space for the heap
	Paging_MapEntry(Paging_directory, Paging_GetPhysicalAddress((uintptr_t)Kernel_dispensary_table), Paging_GetDirectoryEntry(VIRTUAL_MEMORY_START_DISP), PAGING_KERN_TABLE_FLAGS);
	Paging_ClearTable(VIRTUAL_MEMORY_START_DISP);
	Paging_MapVirtualPage(VIRTUAL_MEMORY_START_DISP, PHYSICAL_MEMORY_START_HIGHMEM, PAGING_KERN_PAGE_FLAGS);
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

	Physical_Memory_MakeMap(&Physical_Memory_high, PHYSICAL_MEMORY_START_HIGHMEM + 0x400000, MEMORY_MAX_ADDRESS);

	// Here we set up the map for the free low memory
    Physical_Memory_dma.start  = MEMORY_NULL_PTR;
    Physical_Memory_dma.size   = 0;
    Physical_Memory_dma.attrib = MEMORY_4KB << 8;
    Physical_Memory_dma.node_dispenser = Kernel_node_dispenser;

	Physical_Memory_MakeMap(&Physical_Memory_dma, PHYSICAL_MEMORY_START_DMA, PHYSICAL_MEMORY_START_HIGHMEM);
}


