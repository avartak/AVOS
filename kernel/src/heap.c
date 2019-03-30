#include <kernel/include/memory.h>
#include <kernel/include/physmem.h>
#include <kernel/include/heap.h>

struct          Memory_Map Heap_free;
struct          Memory_Map Heap_inuse;

extern struct   Memory_NodeDispenser* Dispensary_nodepot;

uintptr_t Heap_Allocate(size_t nbytes) {
    struct Memory_Node* node = Memory_Map_Extract(&Heap_free, nbytes);
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
		Memory_Map_Insert(&Heap_free, node, true);
		return (uintptr_t)MEMORY_NULL_PTR;
	}

	Memory_Map_Insert(&Heap_inuse, node, false);

    return node->pointer;
}

bool Heap_Free(uintptr_t pointer) {
    struct Memory_Node* node = Memory_Map_Get(&Heap_inuse, pointer);
	if (node == MEMORY_NULL_PTR) return false;

	uintptr_t start_page = pointer & MEMORY_PAGE_MASK;
	uintptr_t end_page   = (node->pointer + node->size * Memory_Node_GetBaseSize(node->attrib) - 1) & MEMORY_PAGE_MASK;
	
	node->attrib = Heap_free.attrib;
	node->next   = MEMORY_NULL_PTR;
	bool ret_val = Memory_Map_Insert(&Heap_free, node, true);
	if (!ret_val) {
		node->attrib = Heap_inuse.attrib;
    	Memory_Map_Insert(&Heap_inuse, node, false);
		return ret_val; 
	}

	for (uintptr_t page = start_page; page <= end_page; page += MEMORY_SIZE_PAGE) {
		if (Memory_Map_Contains(&Heap_free, page, page + MEMORY_SIZE_PAGE - 1)) Memory_Physical_FreeBlock(page);
	}
	return true;
}

void Heap_Initialize() {
    struct Memory_Node* free_node = Memory_NodeDispenser_Dispense(Dispensary_nodepot);
    Heap_free.start   = free_node;
    Heap_free.size    = MEMORY_END_HEAP - MEMORY_START_HEAP;
    Heap_free.attrib  = MEMORY_1B << 8;
    Heap_free.node_dispenser = Dispensary_nodepot;
   
    free_node->attrib  = Heap_free.attrib;
    free_node->pointer = MEMORY_START_HEAP;
    free_node->size    = Heap_free.size;
    free_node->next    = MEMORY_NULL_PTR;

    free_node = Memory_NodeDispenser_Dispense(Dispensary_nodepot);
    Heap_inuse.start  = MEMORY_NULL_PTR;
    Heap_inuse.size   = 0;
    Heap_inuse.attrib = MEMORY_1B << 8;
    Heap_inuse.node_dispenser = Dispensary_nodepot;
}
