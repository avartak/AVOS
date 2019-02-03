#include <kernel/include/heap.h>

uintptr_t Heap_AllocatePage() {
	struct Memory_Node* node = Memory_Pop(&kernel_heap);
	if (node == MEMORY_NULL_PTR || node->pointer == (uintptr_t)MEMORY_NULL_PTR) return (uintptr_t)MEMORY_NULL_PTR;

	if (Physical_Memory_AllocatePage(node->pointer)) return node->pointer;
	else return (uintptr_t)MEMORY_NULL_PTR;
	
}

bool Heap_FreePage(uintptr_t pointer) {
	if (Physical_Memory_FreePage(pointer)) {
    	struct Memory_Node* node = Memory_NodeDispenser_Dispense(kernel_heap.node_dispenser);
    	if (((uintptr_t)node & 0xFFFFF000) == (pointer & 0xFFFFF000)) return false;
    	node->pointer = pointer;
    	node->size    = 1;
    	node->attrib  = kernel_heap.attrib;
    	node->next    = MEMORY_NULL_PTR;

    	return Memory_Push(&kernel_heap, node);		
	}
	else return false;
}
