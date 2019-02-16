#include <kernel/include/heap.h>

extern struct   Memory_Stack Physical_Memory_high;
struct          Memory_Stack Virtual_Memory_free;
struct          Memory_Stack Virtual_Memory_inuse;

extern bool     Physical_Memory_AllocatePage(uintptr_t virtual_address);
extern bool     Physical_Memory_FreePage(uintptr_t virtual_address);

uintptr_t Heap_Allocate(uint32_t nbytes) {
    struct Memory_Node* node = Memory_Stack_Extract(&Virtual_Memory_free, nbytes);
    if (node == MEMORY_NULL_PTR || node->pointer == (uintptr_t)MEMORY_NULL_PTR) return (uintptr_t)MEMORY_NULL_PTR;

	uintptr_t start_page  = (node->pointer) & PAGE_MASK;
	uintptr_t start_alloc = start_page + MEMORY_SIZE_PAGE;
	if (start_page == node->pointer) start_alloc = start_page;
	uintptr_t end_alloc = (node->pointer + nbytes - 1) & PAGE_MASK;

	bool check_alloc = true;
	uintptr_t unalloc = start_alloc;
	for (uintptr_t page = start_alloc; page <= end_alloc; page += MEMORY_SIZE_PAGE) {
		check_alloc = Physical_Memory_AllocatePage(page);
		if (!check_alloc) {
			unalloc = page;
			break;
		}
	}
	if (!check_alloc) {
		for (uintptr_t page = start_alloc; page <= end_alloc && page <= unalloc; page += MEMORY_SIZE_PAGE) Physical_Memory_FreePage(page);
		Memory_Stack_Insert(&Virtual_Memory_free, node, true);
		return (uintptr_t)MEMORY_NULL_PTR;
	}

	Memory_Stack_Insert(&Virtual_Memory_inuse, node, false);

    return node->pointer;
}

bool Heap_Free(uintptr_t pointer) {
    struct Memory_Node* node = Memory_Stack_Get(&Virtual_Memory_inuse, pointer);
	if (node == MEMORY_NULL_PTR) return false;

	uintptr_t start_page = pointer & PAGE_MASK;
	uintptr_t end_page   = (node->pointer + node->size * Memory_Node_GetBaseSize(node->attrib) - 1) & PAGE_MASK;
	
	node->attrib = Virtual_Memory_free.attrib;
	node->next   = MEMORY_NULL_PTR;
	bool ret_val = Memory_Stack_Insert(&Virtual_Memory_free, node, true);
	if (!ret_val) {
		node->attrib = Virtual_Memory_inuse.attrib;
    	Memory_Stack_Insert(&Virtual_Memory_inuse, node, false);
		return ret_val; 
	}

	for (uintptr_t page = start_page; page <= end_page; page += MEMORY_SIZE_PAGE) {
		if (Memory_Stack_Contains(&Virtual_Memory_free, page, page + MEMORY_SIZE_PAGE - 1)) Physical_Memory_FreePage(page);
	}
	return true;
}

void Heap_Initialize() {
    struct Memory_Node* free_node = Memory_NodeDispenser_Dispense(Physical_Memory_high.node_dispenser);
    Virtual_Memory_free.start  = free_node;
    Virtual_Memory_free.size   = VIRTUAL_MEMORY_END_HEAP - VIRTUAL_MEMORY_START_HEAP;
    Virtual_Memory_free.attrib = MEMORY_1B << 8;
    Virtual_Memory_free.node_dispenser = Physical_Memory_high.node_dispenser;
   
    free_node->attrib  = Virtual_Memory_free.attrib;
    free_node->pointer = VIRTUAL_MEMORY_START_HEAP;
    free_node->size    = Virtual_Memory_free.size;
    free_node->next    = MEMORY_NULL_PTR;

    free_node = Memory_NodeDispenser_Dispense(Physical_Memory_high.node_dispenser);
    Virtual_Memory_inuse.start  = MEMORY_NULL_PTR;
    Virtual_Memory_inuse.size   = 0;
    Virtual_Memory_inuse.attrib = MEMORY_1B << 8;
    Virtual_Memory_inuse.node_dispenser = Physical_Memory_high.node_dispenser;
}
