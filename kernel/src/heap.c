#include <kernel/include/heap.h>

uintptr_t Heap_AllocatePage() {
	struct Memory_Node* node = Memory_Stack_Pop(&Virtual_Memory_free);
	if (node == MEMORY_NULL_PTR || node->pointer == (uintptr_t)MEMORY_NULL_PTR) return (uintptr_t)MEMORY_NULL_PTR;

	if (Physical_Memory_AllocatePage(node->pointer) == false) {
		Memory_Stack_Push(&Virtual_Memory_free, node);
		return (uintptr_t)MEMORY_NULL_PTR;
	}

	if (Memory_Stack_Push(&Virtual_Memory_inuse, node) == false) {
		Physical_Memory_FreePage(node->pointer);
		Memory_Stack_Push(&Virtual_Memory_free, node);
		return (uintptr_t)MEMORY_NULL_PTR;
	}

	return node->pointer;
}

bool Heap_FreePage(uintptr_t pointer) {
	struct Memory_Node* node = Memory_Stack_Get(&Virtual_Memory_inuse, pointer);
	if (node == MEMORY_NULL_PTR) return false;

	if (((uintptr_t)node & 0xFFFFF000) == (pointer & 0xFFFFF000) || Physical_Memory_FreePage(pointer) == false) {
		Memory_Stack_Push(&Virtual_Memory_inuse, node);
		return false;
	}
	
	node->attrib  = Virtual_Memory_free.attrib;
	node->next    = MEMORY_NULL_PTR;
	return Memory_Stack_Push(&Virtual_Memory_free, node);		
}
