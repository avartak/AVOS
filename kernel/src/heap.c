#include <kernel/include/heap.h>

uintptr_t Heap_AllocatePage() {
	struct Memory_Node* node = Memory_Stack_Extract(&Virtual_Memory_free, 0x1000, 0x1000);
	if (node == MEMORY_NULL_PTR || node->pointer == (uintptr_t)MEMORY_NULL_PTR) return (uintptr_t)MEMORY_NULL_PTR;

	if (Physical_Memory_AllocatePage(node->pointer) == false) {
		Memory_Stack_Insert(&Virtual_Memory_free, node, true);
		return (uintptr_t)MEMORY_NULL_PTR;
	}

	if (Memory_Stack_Insert(&Virtual_Memory_inuse, node, false) == false) {
		Physical_Memory_FreePage(node->pointer);
		Memory_Stack_Insert(&Virtual_Memory_free, node, true);
		return (uintptr_t)MEMORY_NULL_PTR;
	}

	return node->pointer;
}

bool Heap_FreePage(uintptr_t pointer) {
	struct Memory_Node* node = Memory_Stack_Get(&Virtual_Memory_inuse, pointer);
	if (node == MEMORY_NULL_PTR) return false;

	if (((uintptr_t)node & 0xFFFFF000) == (pointer & 0xFFFFF000) || Physical_Memory_FreePage(pointer) == false) {
		Memory_Stack_Insert(&Virtual_Memory_inuse, node, false);
		return false;
	}
	
	node->attrib = Virtual_Memory_free.attrib;
	node->next   = MEMORY_NULL_PTR;
	bool ret_val =  Memory_Stack_Insert(&Virtual_Memory_free, node, true);		
	if (!ret_val) {
		node->attrib = Virtual_Memory_inuse.attrib;
    	Memory_Stack_Insert(&Virtual_Memory_inuse, node, false);
	}
	return ret_val;
}

uintptr_t Heap_Allocate(uint32_t nbytes) {
    struct Memory_Node* node = Memory_Stack_Extract(&Virtual_Memory_free, nbytes, 1);
    if (node == MEMORY_NULL_PTR || node->pointer == (uintptr_t)MEMORY_NULL_PTR) return (uintptr_t)MEMORY_NULL_PTR;

	uintptr_t start_page  = (node->pointer) & (~0xFFF);
	uintptr_t start_alloc = start_page + MEMORY_SIZE_PAGE;
	if (start_page == node->pointer) start_alloc = start_page;
	uintptr_t end_alloc = (node->pointer + nbytes) & (~0xFFF);

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
    	for (uintptr_t page = start_alloc; page <= end_alloc && page <= unalloc; page += MEMORY_SIZE_PAGE) {
        	Physical_Memory_FreePage(page);
    	}
		Memory_Stack_Insert(&Virtual_Memory_free, node, true);
		return (uintptr_t)MEMORY_NULL_PTR;
	}

	Memory_Stack_Insert(&Virtual_Memory_inuse, node, false);

    return node->pointer;
}

bool Heap_Free(uintptr_t pointer) {
    struct Memory_Node* node = Memory_Stack_Get(&Virtual_Memory_inuse, pointer);
	if (node == MEMORY_NULL_PTR) return false;

	uintptr_t start_page = pointer & (~0xFFF);
	uintptr_t end_page   = (node->pointer + node->size * Memory_Node_GetBaseSize(node->attrib)) & (~0xFFF);
	
	node->attrib = Virtual_Memory_free.attrib;
	node->next   = MEMORY_NULL_PTR;
	bool ret_val = Memory_Stack_Insert(&Virtual_Memory_free, node, true);
	if (!ret_val) {
		node->attrib = Virtual_Memory_inuse.attrib;
    	Memory_Stack_Insert(&Virtual_Memory_inuse, node, false);
		return ret_val; 
	}

	for (uintptr_t page = start_page; page <= end_page; page += MEMORY_SIZE_PAGE) {
		if (Memory_Stack_Contains(&Virtual_Memory_free, page, page + MEMORY_SIZE_PAGE)) Physical_Memory_FreePage(page);
	}
	return true;
}

