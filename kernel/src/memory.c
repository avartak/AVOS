#include <kernel/include/memory.h>
#include <kernel/include/paging.h>

struct Memory_Map Memory_Physical_free;
struct Memory_Map Memory_Virtual_free;
struct Memory_Map Memory_Virtual_inuse;

uint8_t  Dispensary_pagemap[DISPENSARY_SIZE];
uint32_t Dispensary_pagetable[0x400]__attribute__((aligned(0x1000)));
struct   Memory_NodeDispenser* Dispensary_nodepot = MEMORY_NULL_PTR;

extern uint32_t* Boot_Info_Ptr;

size_t Memory_Node_GetBaseSize(uint32_t attrib) {
    uint8_t  base_size_attrib =  (uint8_t)((attrib & 0xFF00) >> 8);
    if      (base_size_attrib == MEMORY_1B  ) return 1;
    else if (base_size_attrib == MEMORY_4B  ) return 4;
    else if (base_size_attrib == MEMORY_16B ) return 0x10;
    else if (base_size_attrib == MEMORY_512B) return 0x200;
    else if (base_size_attrib == MEMORY_4KB ) return 0x1000;
    else if (base_size_attrib == MEMORY_4MB ) return 0x400000;
    else return 0;
}

uintptr_t Memory_NodeDispenser_New() {
    uintptr_t pointer = (uintptr_t)MEMORY_NULL_PTR;
    uint32_t  ptridx  = 0xFFFFFFFF;

    for (size_t i = 0; i < DISPENSARY_SIZE; i++) {
        if (Dispensary_pagemap[i] == 0) {
            Dispensary_pagemap[i] = 1;
            pointer = MEMORY_START_DISP + i*MEMORY_SIZE_PAGE;
            ptridx  = i;
        }
    }

    if (pointer == (uintptr_t)MEMORY_NULL_PTR) return (uintptr_t)MEMORY_NULL_PTR;
    if (!Memory_Physical_AllocateBlock(pointer)) {
        Dispensary_pagemap[ptridx] = 0;
        return (uintptr_t)MEMORY_NULL_PTR;
    }

    return pointer;
}

bool Memory_NodeDispenser_Delete(uintptr_t pointer) {
    if (pointer < MEMORY_START_DISP || pointer >= MEMORY_END_DISP) return false;
    if (!Memory_Physical_FreeBlock(pointer)) return false;
    Dispensary_pagemap[(pointer - MEMORY_START_DISP)/MEMORY_SIZE_PAGE] = 0;
    return true;
}

size_t Memory_NodeDispenser_NodesLeft(struct Memory_NodeDispenser* dispenser) {
    size_t size = 0;
	for (struct Memory_NodeDispenser* idisp = dispenser; idisp != MEMORY_NULL_PTR; idisp = idisp->next) size += idisp->size;
    return size;
}

size_t Memory_NodeDispenser_FullCount(struct Memory_NodeDispenser* dispenser) {
    size_t count = 0;
	for (struct Memory_NodeDispenser* idisp = dispenser; idisp != MEMORY_NULL_PTR; idisp = idisp->next) {
        if (idisp->size == DISPENSER_FULL_SIZE) count++;
	}
    return count;
}

bool Memory_NodeDispenser_Return(struct Memory_Node* node) {
    if (node == MEMORY_NULL_PTR || (node->attrib | 0xFF) == node->attrib) return false;
    node->attrib |= 0xFF;
    struct Memory_NodeDispenser* dispenser = DISPENSER_FROM_NODE(node);
    if (dispenser->freenode == (uintptr_t)dispenser || dispenser->freenode > (uintptr_t)node) dispenser->freenode = (uintptr_t)node;
    (dispenser->size)++;
    return true;
}

bool Memory_NodeDispenser_Refill(struct Memory_NodeDispenser* dispenser) {
    if (dispenser == MEMORY_NULL_PTR) return false;
    struct Memory_NodeDispenser* new_dispenser = (struct Memory_NodeDispenser*)Memory_NodeDispenser_New();
    if (new_dispenser == MEMORY_NULL_PTR) return false;

    struct Memory_NodeDispenser* current_dispenser = dispenser;
    while (current_dispenser->next != MEMORY_NULL_PTR) current_dispenser = current_dispenser->next;
    current_dispenser->next = new_dispenser;
    new_dispenser->freenode = DISPENSER_FIRST_NODE(new_dispenser);
    new_dispenser->size = DISPENSER_FULL_SIZE;
    new_dispenser->attrib = 0;
    new_dispenser->next = MEMORY_NULL_PTR;
    struct Memory_Node* nodes = (struct Memory_Node*)(DISPENSER_FIRST_NODE(new_dispenser));
    for (size_t i = 0; i < DISPENSER_FULL_SIZE; i++) nodes[i].attrib = 0xFF;
    return true;
}

void Memory_NodeDispenser_Retire(struct Memory_NodeDispenser* dispenser) {
	if (Memory_NodeDispenser_FullCount(dispenser) <= 1) return;

	struct Memory_NodeDispenser* pdisp = dispenser;
	bool found_one_full = false;
	for (struct Memory_NodeDispenser* idisp = dispenser; idisp != MEMORY_NULL_PTR; pdisp = idisp, idisp = idisp->next) {
	    if (idisp->size != DISPENSER_FULL_SIZE) continue;
	    if (!found_one_full) found_one_full = true;
	    else {
	        struct Memory_NodeDispenser* ndisp = idisp->next;
	        if (Memory_NodeDispenser_Delete((uintptr_t)idisp)) pdisp->next = ndisp;
	    }
	}
}

struct Memory_Node* Memory_NodeDispenser_Dispense(struct Memory_NodeDispenser* dispenser) {
	if (dispenser == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;
	size_t nodes_left = Memory_NodeDispenser_NodesLeft(dispenser);
	if (nodes_left == 0) return MEMORY_NULL_PTR;

	struct Memory_NodeDispenser* current_dispenser = dispenser;
	while (current_dispenser != MEMORY_NULL_PTR && (uintptr_t)(current_dispenser->freenode) == (uintptr_t)current_dispenser) current_dispenser = current_dispenser->next;
	if (current_dispenser == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;

	struct Memory_Node* return_node = (struct Memory_Node*)(current_dispenser->freenode);
	return_node->attrib &= (~0xFF);
	(current_dispenser->size)--;
	current_dispenser->freenode = (uintptr_t)current_dispenser;
	struct Memory_Node* nodes = (struct Memory_Node*)DISPENSER_FIRST_NODE(current_dispenser);
	for (size_t i = 0; i < DISPENSER_FULL_SIZE; i++) {
	    if ((nodes[i].attrib | 0xFF) == nodes[i].attrib) current_dispenser->freenode = (uintptr_t)(&(nodes[i]));
	}

	if (nodes_left < DISPENSER_BOTTOM_OUT) Memory_NodeDispenser_Refill(dispenser);
	return return_node;
}

bool Memory_Map_Contains(struct Memory_Map* stack, uintptr_t ptr_min, uintptr_t ptr_max) {
    if (stack == MEMORY_NULL_PTR) return false;
	for (struct Memory_Node* imnode = stack->start; imnode != MEMORY_NULL_PTR; imnode = imnode->next) {
		if (ptr_min >= imnode->pointer && ptr_max <= imnode->pointer + imnode->size * Memory_Node_GetBaseSize(stack->attrib)) return true;
	}
	return false;
}

bool Memory_Map_Push(struct Memory_Map* stack, struct Memory_Node* node, bool merge) {
    if (stack == MEMORY_NULL_PTR || node == MEMORY_NULL_PTR) return false;
    if (node->attrib != stack->attrib) return false;

    size_t node_base_size = Memory_Node_GetBaseSize(node->attrib);
    uintptr_t node_end = node->pointer + node->size * node_base_size;
    if (node_end <= node->pointer) return false;

    if (stack->start == MEMORY_NULL_PTR || node_end != stack->start->pointer || !merge) {
        node->next = stack->start;
        stack->start = node;
    }
    else {
        stack->start->size += node->size;
        stack->start->pointer = node->pointer;
        Memory_NodeDispenser_Return(node);
        Memory_NodeDispenser_Retire(stack->node_dispenser);
    }

    stack->size += node->size;
    return true;
}

bool Memory_Map_Append(struct Memory_Map* stack, struct Memory_Node* node, bool merge) {
    if (stack == MEMORY_NULL_PTR || node == MEMORY_NULL_PTR) return false;
    if (node->attrib != stack->attrib) return false;

    size_t node_base_size = Memory_Node_GetBaseSize(stack->attrib);
    uintptr_t node_end = node->pointer + node->size * node_base_size;
    if (node_end <= node->pointer) return false;

	struct Memory_Node* last_node = stack->start;
	if (last_node == MEMORY_NULL_PTR) return Memory_Map_Push(stack, node, merge);
	while (last_node->next != MEMORY_NULL_PTR) last_node = last_node->next;

	if (merge && last_node->pointer + last_node->size * node_base_size == node->pointer) {
        last_node->size += node->size;
        Memory_NodeDispenser_Return(node);
        Memory_NodeDispenser_Retire(stack->node_dispenser);
	}
	else last_node->next = node;

    stack->size += node->size;
    return true;
}

bool Memory_Map_Insert(struct Memory_Map* stack, struct Memory_Node* node, bool merge) {
    if (stack == MEMORY_NULL_PTR || node == MEMORY_NULL_PTR) return false;
    if (node->attrib != stack->attrib) return false;

    size_t node_base_size = Memory_Node_GetBaseSize(stack->attrib);
    uintptr_t node_end = node->pointer + node->size * node_base_size;
    if (node_end <= node->pointer) return false;

	if (stack->start == MEMORY_NULL_PTR || node_end <= stack->start->pointer) return Memory_Map_Push(stack, node, merge);

	struct Memory_Node* current_node = stack->start;
	while (current_node->next != MEMORY_NULL_PTR) {
		if (current_node->pointer + current_node->size * node_base_size < node->pointer && current_node->next->pointer > node_end) {
			struct Memory_Node* next = current_node->next;
			current_node->next = node;
			node->next = next;
			stack->size += node->size;
			return true;
		}
		else if (current_node->pointer + current_node->size * node_base_size == node->pointer && current_node->next->pointer == node_end && merge) {
			current_node->size += current_node->next->size + node->size;
			struct Memory_Node* next = current_node->next->next;
			current_node->next = next;
			stack->size += node->size;
			Memory_NodeDispenser_Return(node);
			Memory_NodeDispenser_Return(current_node->next);
        	Memory_NodeDispenser_Retire(stack->node_dispenser);
			return true;
		}
		else if (current_node->pointer + current_node->size * node_base_size == node->pointer && current_node->next->pointer > node_end && merge) {
			current_node->size += node->size;
			stack->size += node->size;
			Memory_NodeDispenser_Return(node);
			Memory_NodeDispenser_Retire(stack->node_dispenser);
			return true;
		}
		else if (current_node->pointer + current_node->size * node_base_size < node->pointer && current_node->next->pointer == node_end && merge) {
			current_node->next->pointer = node->pointer;
			current_node->next->size += node->size;
			stack->size += node->size;
			Memory_NodeDispenser_Return(node);
			Memory_NodeDispenser_Retire(stack->node_dispenser);
			return true;
		}
		current_node = current_node->next;
	}

	if (current_node->next == MEMORY_NULL_PTR && current_node->pointer + current_node->size * node_base_size <= node->pointer) return Memory_Map_Append(stack, node, merge);

	return false;
}

struct Memory_Node* Memory_Map_Pop(struct Memory_Map* stack) {
    if (stack == MEMORY_NULL_PTR || stack->start == MEMORY_NULL_PTR || stack->size == 0 || stack->start->size == 0) return MEMORY_NULL_PTR;

    struct Memory_Node* current = stack->start;
    size_t base_size = Memory_Node_GetBaseSize(stack->attrib);

	struct Memory_Node* pop_node = MEMORY_NULL_PTR;
    if (current->size == 1) {
        pop_node = current;
        stack->start = current->next;
		pop_node->next = MEMORY_NULL_PTR;
    }

    else {	
		pop_node = Memory_NodeDispenser_Dispense(stack->node_dispenser);
		if (pop_node == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;
        pop_node->pointer = current->pointer;
		pop_node->size = 1;
		pop_node->attrib = stack->attrib;
		pop_node->next = MEMORY_NULL_PTR;

        current->pointer += base_size;
        (current->size)--;
    }

	(stack->size)--;
	return pop_node;
}

struct Memory_Node* Memory_Map_Extract(struct Memory_Map* stack, size_t node_size) {
    if (stack == MEMORY_NULL_PTR || stack->start == MEMORY_NULL_PTR || node_size == 0 || stack->size < node_size) return MEMORY_NULL_PTR;

    struct Memory_Node* pop_node = MEMORY_NULL_PTR;
    struct Memory_Node* current  = stack->start;
    size_t base_size             = Memory_Node_GetBaseSize(stack->attrib);

	if (stack->start->size == node_size) {
		pop_node = stack->start;
		stack->start = stack->start->next;
		pop_node->next = MEMORY_NULL_PTR;

    	(stack->size) -= node_size;
    	return pop_node;
	}
	else if (stack->start->size > node_size) {
		pop_node = Memory_NodeDispenser_Dispense(stack->node_dispenser);
		if (pop_node == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;
		pop_node->pointer = stack->start->pointer;
		pop_node->size = node_size;
		pop_node->attrib = stack->attrib;
		pop_node->next = MEMORY_NULL_PTR;
		
		stack->start->pointer += node_size * base_size;
		(stack->start->size)  -= node_size;
		
		(stack->size) -= node_size;
		return pop_node;
	}

	while (current->next != MEMORY_NULL_PTR) {
		if (current->next->size < node_size) {
			current = current->next;
		    continue;
		}
		else if (current->next->size == node_size) {
			pop_node = current->next;
			current->next  = current->next->next;
			pop_node->next = MEMORY_NULL_PTR;

    		(stack->size) -= node_size;
    		return pop_node;
		}
        else if (current->next->size > node_size) {
		    pop_node = Memory_NodeDispenser_Dispense(stack->node_dispenser);
		    if (pop_node == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;

		    pop_node->pointer = current->next->pointer;
		    pop_node->size = node_size;
		    pop_node->attrib = stack->attrib;
		    pop_node->next = MEMORY_NULL_PTR;

			current->next->pointer += node_size * base_size;
			(current->next->size)  -= node_size;
		
    		(stack->size) -= node_size;
            return pop_node;
        } 	
	}

    return pop_node;
}

struct Memory_Node* Memory_Map_Get(struct Memory_Map* stack, uintptr_t node_ptr) {
    if (stack == MEMORY_NULL_PTR || stack->start == MEMORY_NULL_PTR || node_ptr == (uintptr_t)MEMORY_NULL_PTR) return MEMORY_NULL_PTR;

	struct Memory_Node* return_node = MEMORY_NULL_PTR;
    if (stack->start->pointer == node_ptr) {
        return_node  = stack->start;
        stack->start = stack->start->next;
        return_node->next = MEMORY_NULL_PTR;

        (stack->size) -= Memory_Node_GetBaseSize(stack->attrib) * return_node->size;
        return return_node;
    }

    struct Memory_Node* current = stack->start;
    while (current->next != MEMORY_NULL_PTR) {
        if (current->next->pointer != node_ptr) {
            current = current->next;
            continue;
        }
        else {
			return_node = current->next;
			current->next = return_node->next;
			return_node->next = MEMORY_NULL_PTR;

            (stack->size) -= Memory_Node_GetBaseSize(stack->attrib) * return_node->size;
            return return_node;
        }
    }

    return return_node;
}

void Memory_Physical_MakeMap(struct Memory_Map* mem_map, uintptr_t mem_start, uintptr_t mem_end) {
	size_t last_idx = 0;
	while (Boot_Info_Ptr[last_idx] != 0xFFFFFFFF) last_idx++;

    struct Memory_RAM_Table_Entry* table_ptr = (struct Memory_RAM_Table_Entry*)(Boot_Info_Ptr[last_idx-2]+KERNEL_HIGHER_HALF_OFFSET);
    if (mem_start < table_ptr[0].pointer) return;
    for (size_t i = 0; i < Boot_Info_Ptr[last_idx-1]/sizeof(struct Memory_RAM_Table_Entry) && table_ptr[i].pointer < mem_end; i++) {
        struct Memory_Node* node = Memory_NodeDispenser_Dispense(mem_map->node_dispenser);
        if (node == MEMORY_NULL_PTR) return;
        node->pointer = table_ptr[i].pointer;
        node->size    = (mem_end > table_ptr[i].pointer + table_ptr[i].size ? table_ptr[i].size : mem_end - table_ptr[i].pointer) / MEMORY_SIZE_PAGE;
        node->attrib  = mem_map->attrib;
        node->next    = MEMORY_NULL_PTR;
        Memory_Map_Insert(mem_map, node, true);
    }
}

bool Memory_Physical_AllocateBlock(uintptr_t virtual_address) {
	struct Memory_Node* mem_node = Memory_Map_Pop(&Memory_Physical_free);
	if (mem_node == MEMORY_NULL_PTR) return false;

	if (!Paging_TableExists(virtual_address)) {
		struct Memory_Node* table_node = Memory_Map_Pop(&Memory_Physical_free);
		if (table_node == MEMORY_NULL_PTR) {
			Memory_Map_Push(&Memory_Physical_free, mem_node, true);
			return false;
		}
		Paging_MapEntry(Paging_directory, table_node->pointer, Paging_GetDirectoryEntry(virtual_address), PAGING_KERN_TABLE_FLAGS);
		Paging_ClearTable(virtual_address);
		Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_directory));
	}
	
	if (!(Paging_MapVirtualPage(virtual_address, mem_node->pointer, PAGING_KERN_PAGE_FLAGS))) {
		Memory_Map_Push(&Memory_Physical_free, mem_node, true);
		return false;
	}

	return true;
}

bool Memory_Physical_AllocateBlocks(uintptr_t virtual_address, size_t nblocks) {
    struct Memory_Node* mem_node = Memory_Map_Extract(&Memory_Physical_free, nblocks);
    if (mem_node == MEMORY_NULL_PTR) return false;

	size_t iblock;
	bool alloc_status = true;
	for (iblock = 0; iblock < nblocks; iblock++) {
		uintptr_t ivirt_addr = virtual_address + iblock * MEMORY_SIZE_PAGE;
		if (!Paging_TableExists(ivirt_addr)) {
		    struct Memory_Node* table_node = Memory_Map_Pop(&Memory_Physical_free);
		    if (table_node == MEMORY_NULL_PTR) {
		        Memory_Map_Insert(&Memory_Physical_free, mem_node, true);
		        alloc_status = false;
				break;
		    }
		    Paging_MapEntry(Paging_directory, table_node->pointer, Paging_GetDirectoryEntry(ivirt_addr), PAGING_KERN_TABLE_FLAGS);
		    Paging_ClearTable(ivirt_addr);
		    Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_directory));
		}
		
		if (!(Paging_MapVirtualPage(ivirt_addr, mem_node->pointer, PAGING_KERN_PAGE_FLAGS))) {
		    Memory_Map_Insert(&Memory_Physical_free, mem_node, true);
		    alloc_status = false;
			break;
		}
	}

	if (!alloc_status) {
		for (size_t i = 0; i < iblock; i++) Paging_UnmapVirtualPage(virtual_address + i*MEMORY_SIZE_PAGE);
	}

    return alloc_status;
}

bool Memory_Physical_FreeBlock(uintptr_t virtual_address) {
	struct Memory_Node* node = Memory_NodeDispenser_Dispense(Memory_Physical_free.node_dispenser);
	uintptr_t physical_address = Paging_GetPhysicalAddress(virtual_address);

	if (((uintptr_t)node & MEMORY_PAGE_MASK) == (virtual_address & MEMORY_PAGE_MASK) || !(Paging_UnmapVirtualPage(virtual_address))) {
		Memory_NodeDispenser_Return(node);
		return false;
	}

	node->pointer = physical_address;
	node->size    = 1;
	node->attrib  = Memory_Physical_free.attrib;
	node->next    = MEMORY_NULL_PTR;
	
	return Memory_Map_Insert(&Memory_Physical_free, node, true);
}

bool Memory_Physical_FreeBlocks(uintptr_t virtual_address, size_t nblocks) {
	bool retval = true;
	for (size_t i = 0; i < nblocks; i++) retval = retval && Memory_Physical_FreeBlock(virtual_address + i*MEMORY_SIZE_PAGE);
	return retval;
}

uintptr_t Memory_Virtual_Allocate(size_t nbytes) {
    struct Memory_Node* node = Memory_Map_Extract(&Memory_Virtual_free, nbytes);
    if (node == MEMORY_NULL_PTR || node->pointer == (uintptr_t)MEMORY_NULL_PTR) return (uintptr_t)MEMORY_NULL_PTR;

	uintptr_t start_page  = (node->pointer) & MEMORY_PAGE_MASK;
	uintptr_t start_alloc = start_page + MEMORY_SIZE_PAGE;
	if (start_page == node->pointer) start_alloc = start_page;
	uintptr_t end_alloc = (node->pointer + nbytes - 1) & MEMORY_PAGE_MASK;

	bool check_alloc = true;
	uintptr_t unalloc = start_alloc;
	for (uintptr_t page = start_alloc; page <= end_alloc; page += MEMORY_SIZE_PAGE) {
		check_alloc = Memory_Physical_AllocateBlock(page);
		if (!check_alloc) {
			unalloc = page;
			break;
		}
	}
	if (!check_alloc) {
		for (uintptr_t page = start_alloc; page <= end_alloc && page <= unalloc; page += MEMORY_SIZE_PAGE) Memory_Physical_FreeBlock(page);
		Memory_Map_Insert(&Memory_Virtual_free, node, true);
		return (uintptr_t)MEMORY_NULL_PTR;
	}

	Memory_Map_Insert(&Memory_Virtual_inuse, node, false);

    return node->pointer;
}

bool Memory_Virtual_Free(uintptr_t pointer) {
    struct Memory_Node* node = Memory_Map_Get(&Memory_Virtual_inuse, pointer);
	if (node == MEMORY_NULL_PTR) return false;

	uintptr_t start_page = pointer & MEMORY_PAGE_MASK;
	uintptr_t end_page   = (node->pointer + node->size * Memory_Node_GetBaseSize(node->attrib) - 1) & MEMORY_PAGE_MASK;
	
	node->attrib = Memory_Virtual_free.attrib;
	node->next   = MEMORY_NULL_PTR;
	bool ret_val = Memory_Map_Insert(&Memory_Virtual_free, node, true);
	if (!ret_val) {
		node->attrib = Memory_Virtual_inuse.attrib;
    	Memory_Map_Insert(&Memory_Virtual_inuse, node, false);
		return ret_val; 
	}

	for (uintptr_t page = start_page; page <= end_page; page += MEMORY_SIZE_PAGE) {
		if (Memory_Map_Contains(&Memory_Virtual_free, page, page + MEMORY_SIZE_PAGE - 1)) Memory_Physical_FreeBlock(page);
	}
	return true;
}

void Memory_Initialize() {

    // Lets create the virtual space for the heap
    Paging_MapEntry(Paging_directory, Paging_GetPhysicalAddress((uintptr_t)Dispensary_pagetable), Paging_GetDirectoryEntry(MEMORY_START_DISP), PAGING_KERN_TABLE_FLAGS);
    Paging_ClearTable(MEMORY_START_DISP);
    Paging_MapVirtualPage(MEMORY_START_DISP, MEMORY_PHYSICAL_START_HIGHMEM, PAGING_KERN_PAGE_FLAGS);
    Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_directory));

    // Lets setup the node dispenser at the very start of the heap
    Dispensary_nodepot = (struct Memory_NodeDispenser*)DISPENSER_PRIME_ADDRESS;
    Dispensary_nodepot->freenode = DISPENSER_FIRST_NODE(Dispensary_nodepot);
    Dispensary_nodepot->size = DISPENSER_FULL_SIZE;
    Dispensary_nodepot->attrib = 0;
    Dispensary_nodepot->next = MEMORY_NULL_PTR;
    struct Memory_Node* nodes = (struct Memory_Node*)DISPENSER_FIRST_NODE(Dispensary_nodepot);
    for (size_t i = 0; i < Dispensary_nodepot->size; i++) nodes[i].attrib = Memory_Physical_free.attrib | 0xFF;

    // Here we initialize the bitmap
    for (size_t i = 0; i < DISPENSARY_SIZE; i++) Dispensary_pagemap[i] = 0;

    // Here we set up the map for the physical memory
    Memory_Physical_free.start  = MEMORY_NULL_PTR;
    Memory_Physical_free.size   = 0;
    Memory_Physical_free.attrib = MEMORY_4KB << 8;
    Memory_Physical_free.node_dispenser = Dispensary_nodepot;
    Memory_Physical_MakeMap(&Memory_Physical_free, MEMORY_PHYSICAL_START_HIGHMEM, MEMORY_MAX_ADDRESS);

    // Here we set up the map for the virtual free memory
    struct Memory_Node* free_node = Memory_NodeDispenser_Dispense(Dispensary_nodepot);
    Memory_Virtual_free.start   = free_node;
    Memory_Virtual_free.size    = MEMORY_END_HEAP - MEMORY_START_HEAP;
    Memory_Virtual_free.attrib  = MEMORY_1B << 8;
    Memory_Virtual_free.node_dispenser = Dispensary_nodepot;
   
    free_node->attrib  = Memory_Virtual_free.attrib;
    free_node->pointer = MEMORY_START_HEAP;
    free_node->size    = Memory_Virtual_free.size;
    free_node->next    = MEMORY_NULL_PTR;

    // Here we set up the map for the virtual memory in use
    Memory_Virtual_inuse.start  = MEMORY_NULL_PTR;
    Memory_Virtual_inuse.size   = 0;
    Memory_Virtual_inuse.attrib = MEMORY_1B << 8;
    Memory_Virtual_inuse.node_dispenser = Dispensary_nodepot;

}
