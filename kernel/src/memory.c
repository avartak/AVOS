#include <kernel/include/memory.h>
#include <kernel/include/heap.h>

#include <stddef.h>

uintptr_t Memory_AllocatePage() {
	return Heap_AllocatePage();
}

bool Memory_FreePage(uintptr_t pointer) {
	return Heap_FreePage(pointer);
}

uint32_t Memory_NodeDispenser_NodesLeft(struct Memory_NodeDispenser* dispenser) {
	if (dispenser == MEMORY_NULL_PTR) return 0;
	struct Memory_NodeDispenser* current_dispenser = dispenser;
	uint32_t size = 0;
	while (current_dispenser != MEMORY_NULL_PTR) {
		size += current_dispenser->size;
		current_dispenser = current_dispenser->next;
	}
	return size;
}

bool Memory_NodeDispenser_Return(struct Memory_Node* node) {
	if (node == MEMORY_NULL_PTR) return false;
	node->attrib |= 0x000000FF; 
	struct Memory_NodeDispenser* dispenser = (struct Memory_NodeDispenser*)((uintptr_t)node & 0xFFFFF000);
	if (dispenser->freenode == (uintptr_t)dispenser) dispenser->freenode = (uintptr_t)node;
	else if (dispenser->freenode > (uintptr_t)node)  dispenser->freenode = (uintptr_t)node;
	(dispenser->size)++;	
	return true;
}

bool Memory_NodeDispenser_Refill(struct Memory_NodeDispenser* dispenser) {
	if (dispenser == MEMORY_NULL_PTR) return false;
	struct Memory_NodeDispenser* new_dispenser = (struct Memory_NodeDispenser*)Memory_AllocatePage();
	if (new_dispenser == MEMORY_NULL_PTR) return false;

	struct Memory_NodeDispenser* current_dispenser = dispenser;
	while (current_dispenser->next != MEMORY_NULL_PTR) current_dispenser = current_dispenser->next;
	current_dispenser->next = new_dispenser;
	new_dispenser->freenode = (uintptr_t)new_dispenser + sizeof(struct Memory_NodeDispenser);
	new_dispenser->size = (MEMORY_SIZE_PAGE - sizeof(struct Memory_NodeDispenser)) / sizeof(struct Memory_Node); 
	new_dispenser->attrib = 0;
	new_dispenser->next = MEMORY_NULL_PTR;
	struct Memory_Node* nodes = (struct Memory_Node*)(new_dispenser + sizeof(struct Memory_NodeDispenser));
	for (size_t i = 0; i < new_dispenser->size; i++) nodes[i].attrib |= 0x000000FF;
	return true;
}

bool Memory_NodeDispenser_Retire(struct Memory_NodeDispenser* dispenser) {
	if (dispenser == MEMORY_NULL_PTR) return false;
	if (dispenser->size == (MEMORY_SIZE_PAGE - sizeof(struct Memory_NodeDispenser)) / sizeof(struct Memory_Node) && dispenser->freenode != (uintptr_t)dispenser) return Memory_FreePage((uintptr_t)dispenser);
	else return false;
}

struct Memory_Node* Memory_NodeDispenser_Dispense(struct Memory_NodeDispenser* dispenser) {
	if (dispenser == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;

	uint32_t nodes_left = Memory_NodeDispenser_NodesLeft(dispenser);
	if (nodes_left < MIN_NODES_B4_REFILL) {
		if (Memory_NodeDispenser_Refill(dispenser)) return Memory_NodeDispenser_Dispense(dispenser);
		else return MEMORY_NULL_PTR;
	}
	else {
		struct Memory_NodeDispenser* current_dispenser = dispenser;
		while (current_dispenser != MEMORY_NULL_PTR && (uintptr_t)(current_dispenser->freenode) == (uintptr_t)current_dispenser) current_dispenser = current_dispenser->next;
		if (current_dispenser == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;
		else {
			struct Memory_Node* return_node = (struct Memory_Node*)(current_dispenser->freenode);
			return_node->attrib &= 0xFFFFFF00;
			current_dispenser->freenode = (uintptr_t)current_dispenser;
			struct Memory_Node* nodes = (struct Memory_Node*)((uintptr_t)current_dispenser + sizeof(struct Memory_NodeDispenser));
			for (size_t i = 0; i < (MEMORY_SIZE_PAGE - sizeof(struct Memory_NodeDispenser)) / sizeof(struct Memory_Node); i++) {
				if ((nodes[i].attrib & 0x000000FF) == 0x000000FF) current_dispenser->freenode = (uintptr_t)(&(nodes[i]));
			}
			(current_dispenser->size)--;
			return return_node;
		}
	}
}

bool Memory_Push(struct Memory_FIFO* stack, struct Memory_Node* node) {
    if (stack == MEMORY_NULL_PTR || node == MEMORY_NULL_PTR) return false;
    if (node->attrib != stack->attrib) return false;

    uint32_t node_base_size = Memory_GetBaseSize(node->attrib);
    uint32_t node_end = node->pointer + node->size * node_base_size;
    if (node_end <= node->pointer) return false;

    if (stack->start == MEMORY_NULL_PTR || node_end != stack->start->pointer) {
        node->next = stack->start;
        stack->start = node;
    }
    else {
        stack->start->size += node->size;
        stack->start->pointer = node->pointer;
        Memory_NodeDispenser_Return(node);
    }

    stack->size += node->size;
    return true;
}

bool Memory_Append(struct Memory_FIFO* stack, struct Memory_Node* node) {
    if (stack == MEMORY_NULL_PTR || node == MEMORY_NULL_PTR) return false;
    if (node->attrib != stack->attrib) return false;

    uint32_t node_base_size = Memory_GetBaseSize(stack->attrib);
    uint32_t node_end = node->pointer + node->size * node_base_size;
    if (node_end <= node->pointer) return false;

	struct Memory_Node* last_node = stack->start;
	if (last_node == MEMORY_NULL_PTR) return Memory_Push(stack, node);
	while (last_node->next != MEMORY_NULL_PTR) last_node = last_node->next;

    if (last_node->pointer + last_node->size * node_base_size && node_end != node->pointer) {
        last_node->next = node;
    }
    else {
        last_node->size += node->size;
        Memory_NodeDispenser_Return(node);
    }

    stack->size += node->size;
    return true;
}

struct Memory_Node* Memory_Pop(struct Memory_FIFO* stack) {
    if (stack == MEMORY_NULL_PTR || stack->size == 0) return MEMORY_NULL_PTR;

    struct Memory_Node* current = stack->start;
    if (current == MEMORY_NULL_PTR || current->size == 0) return MEMORY_NULL_PTR;
    uint32_t current_base_size = Memory_GetBaseSize(stack->attrib);
    if (current->pointer + current_base_size <= current->pointer) return MEMORY_NULL_PTR;

	struct Memory_Node* pop_node = MEMORY_NULL_PTR;
    if (current->size == 1) {
        pop_node = current;
        stack->start = current->next;
    }

    else {	
		pop_node = Memory_NodeDispenser_Dispense(stack->node_dispenser);
		if (pop_node == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;
        pop_node->pointer = current->pointer;
		pop_node->size = 1;
		pop_node->attrib = stack->attrib;
		pop_node->next = MEMORY_NULL_PTR;

        current->pointer += current_base_size;
        (current->size)--;
    }

	(stack->size)--;
	return pop_node;
}


