#include <kernel/include/chunk.h>

uintptr_t Chunk_AllocatePage() {
	struct Memory_Node* node = Memory_Stack_Pop(&Virtual_Memory_chunk);
	if (node == MEMORY_NULL_PTR || node->pointer == (uintptr_t)MEMORY_NULL_PTR) return (uintptr_t)MEMORY_NULL_PTR;

	if (Physical_Memory_AllocatePage(node->pointer)) return node->pointer;
	else return (uintptr_t)MEMORY_NULL_PTR;
	
}

bool Chunk_FreePage(uintptr_t pointer) {
	if (Physical_Memory_FreePage(pointer)) {
    	struct Memory_Node* node = Memory_NodeDispenser_Dispense(Virtual_Memory_chunk.node_dispenser);
    	if (((uintptr_t)node & 0xFFFFF000) == (pointer & 0xFFFFF000)) return false;
    	node->pointer = pointer;
    	node->size    = 1;
    	node->attrib  = Virtual_Memory_chunk.attrib;
    	node->next    = MEMORY_NULL_PTR;

    	return Memory_Stack_Push(&Virtual_Memory_chunk, node);		
	}
	else return false;
}
