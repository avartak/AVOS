#include <kernel/include/memory.h>
#include <kernel/include/chunk.h>

#include <stddef.h>

uint32_t Memory_Node_GetBaseSize(uint32_t attrib) {
	uint8_t  base_size_attrib =  (uint8_t)(attrib & 0xFF00 >> 8);
	if      (base_size_attrib == 0x1 ) return 1;
	else if (base_size_attrib == 0x2 ) return 4;
	else if (base_size_attrib == 0x4 ) return 0x10;
	else if (base_size_attrib == 0x8 ) return 0x200;
	else if (base_size_attrib == 0x10) return 0x1000;
	else if (base_size_attrib == 0x20) return 0x400000;
	else return 0;
}

uintptr_t Memory_AllocatePage() {
	return Chunk_AllocatePage();
}

bool Memory_FreePage(uintptr_t pointer) {
	return Chunk_FreePage(pointer);
}

uint32_t Memory_NodeDispenser_NodesLeft(struct Memory_NodeDispenser* dispenser) {
	uint32_t size = 0;
	struct Memory_NodeDispenser* current_dispenser = dispenser;
	while (current_dispenser != MEMORY_NULL_PTR) {
		size += current_dispenser->size;
		current_dispenser = current_dispenser->next;
	}
	return size;
}

uint32_t Memory_NodeDispenser_FullCount(struct Memory_NodeDispenser* dispenser) {
    uint32_t count = 0;
    struct Memory_NodeDispenser* current_dispenser = dispenser;
    while (current_dispenser != MEMORY_NULL_PTR) {
        if (current_dispenser->size == FULL_DISPENSER_SIZE) count++;
        current_dispenser = current_dispenser->next;
    }
    return count;
}

bool Memory_NodeDispenser_Return(struct Memory_Node* node) {
	if (node == MEMORY_NULL_PTR || (node->attrib | 0xFF) == node->attrib) return false;
	node->attrib |= 0xFF; 
	struct Memory_NodeDispenser* dispenser = DISPENSER_FROM_NODE(node);
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
	new_dispenser->freenode = ((uintptr_t)new_dispenser) + sizeof(struct Memory_NodeDispenser);
	new_dispenser->size = FULL_DISPENSER_SIZE; 
	new_dispenser->attrib = 0;
	new_dispenser->next = MEMORY_NULL_PTR;
	struct Memory_Node* nodes = (struct Memory_Node*)(new_dispenser->freenode);
	for (size_t i = 0; i < new_dispenser->size; i++) nodes[i].attrib = 0xFF;
	return true;
}

void Memory_NodeDispenser_Retire(struct Memory_NodeDispenser* dispenser) {
    if (Memory_NodeDispenser_FullCount(dispenser) > 1) {
        bool foundone = false;
        struct Memory_NodeDispenser* current_dispenser = dispenser;
        while (current_dispenser != MEMORY_NULL_PTR) {
            if (current_dispenser->size == FULL_DISPENSER_SIZE) {
                if (!foundone) {
                    foundone = true;
                    current_dispenser = current_dispenser->next;
                }
                else {
                    struct Memory_NodeDispenser* disp = dispenser;
                    while (disp->next != current_dispenser) disp = disp->next;
					struct Memory_NodeDispenser* next = current_dispenser->next;
                    bool freed = Memory_FreePage((uintptr_t)current_dispenser);
					if (freed) {
						disp->next = next;
                    	current_dispenser = next;
					}
                }
            }
        }
    }
}

struct Memory_Node* Memory_NodeDispenser_Dispense(struct Memory_NodeDispenser* dispenser) {
	if (dispenser == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;

	uint32_t nodes_left = Memory_NodeDispenser_NodesLeft(dispenser);
	if (nodes_left == 0) return MEMORY_NULL_PTR;
	else {
		struct Memory_NodeDispenser* current_dispenser = dispenser;
		while (current_dispenser != MEMORY_NULL_PTR && (uintptr_t)(current_dispenser->freenode) == (uintptr_t)current_dispenser) current_dispenser = current_dispenser->next;
		if (current_dispenser == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;
		else {
			struct Memory_Node* return_node = (struct Memory_Node*)(current_dispenser->freenode);
			return_node->attrib &= (~0xFF);
			current_dispenser->freenode = (uintptr_t)current_dispenser;
			struct Memory_Node* nodes = (struct Memory_Node*)((uintptr_t)current_dispenser + sizeof(struct Memory_NodeDispenser));
			for (size_t i = 0; i < FULL_DISPENSER_SIZE; i++) {
				if ((nodes[i].attrib | 0xFF) == nodes[i].attrib) current_dispenser->freenode = (uintptr_t)(&(nodes[i]));
			}
			(current_dispenser->size)--;
			if (nodes_left < MIN_NODES_B4_REFILL) Memory_NodeDispenser_Refill(dispenser);
			return return_node;
		}
	}
}

bool Memory_Stack_Push(struct Memory_Stack* stack, struct Memory_Node* node) {
    if (stack == MEMORY_NULL_PTR || node == MEMORY_NULL_PTR) return false;
    if (node->attrib != stack->attrib) return false;

    uint32_t node_base_size = Memory_Node_GetBaseSize(node->attrib);
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
        Memory_NodeDispenser_Retire(stack->node_dispenser);
    }

    stack->size += node->size;
    return true;
}

bool Memory_Stack_Append(struct Memory_Stack* stack, struct Memory_Node* node) {
    if (stack == MEMORY_NULL_PTR || node == MEMORY_NULL_PTR) return false;
    if (node->attrib != stack->attrib) return false;

    uint32_t node_base_size = Memory_Node_GetBaseSize(stack->attrib);
    uint32_t node_end = node->pointer + node->size * node_base_size;
    if (node_end <= node->pointer) return false;

	struct Memory_Node* last_node = stack->start;
	if (last_node == MEMORY_NULL_PTR) return Memory_Stack_Push(stack, node);
	while (last_node->next != MEMORY_NULL_PTR) last_node = last_node->next;

    if (last_node->pointer + last_node->size * node_base_size != node->pointer) {
        last_node->next = node;
    }
    else {
        last_node->size += node->size;
        Memory_NodeDispenser_Return(node);
        Memory_NodeDispenser_Retire(stack->node_dispenser);
    }

    stack->size += node->size;
    return true;
}

bool Memory_Stack_Insert(struct Memory_Stack* stack, struct Memory_Node* node) {
    if (stack == MEMORY_NULL_PTR || node == MEMORY_NULL_PTR) return false;
    if (node->attrib != stack->attrib) return false;

    uint32_t node_base_size = Memory_Node_GetBaseSize(stack->attrib);
    uint32_t node_end = node->pointer + node->size * node_base_size;
    if (node_end <= node->pointer) return false;

	if (stack->start == MEMORY_NULL_PTR || node_end <= stack->start->pointer) return Memory_Stack_Push(stack, node);

	struct Memory_Node* current_node = stack->start;
	while (current_node->next != MEMORY_NULL_PTR) {
		if (current_node->pointer + current_node->size * node_base_size < node->pointer && current_node->next->pointer > node_end) {
			struct Memory_Node* next = current_node->next;
			current_node->next = node;
			node->next = next;
			stack->size += node->size;
			return true;
		}
		else if (current_node->pointer + current_node->size * node_base_size == node->pointer && current_node->next->pointer == node_end) {
			current_node->size += current_node->next->size + node->size;
			struct Memory_Node* next = current_node->next->next;
			Memory_NodeDispenser_Return(node);
			Memory_NodeDispenser_Return(current_node->next);
			current_node->next = next;
			stack->size += node->size;
			return true;
		}
	}

	if (current_node->next == MEMORY_NULL_PTR && current_node->pointer + current_node->size * node_base_size <= node->pointer) return Memory_Stack_Append(stack, node);

	return false;
}

struct Memory_Node* Memory_Stack_Pop(struct Memory_Stack* stack) {
    if (stack == MEMORY_NULL_PTR || stack->start == MEMORY_NULL_PTR || stack->size == 0 || stack->start->size == 0) return MEMORY_NULL_PTR;

    struct Memory_Node* current = stack->start;
    uint32_t base_size = Memory_Node_GetBaseSize(stack->attrib);

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

struct Memory_Node* Memory_Stack_Extract(struct Memory_Stack* stack, uint32_t node_size) {
    if (stack == MEMORY_NULL_PTR || stack->start == MEMORY_NULL_PTR || node_size == 0 || stack->size < node_size) return MEMORY_NULL_PTR;

    struct Memory_Node* pop_node = MEMORY_NULL_PTR;
    struct Memory_Node* current  = stack->start;
    uint32_t base_size           = Memory_Node_GetBaseSize(stack->attrib);

	if (stack->start->size == node_size) {
		pop_node = stack->start;
		stack->start = stack->start->next;
		pop_node->next = MEMORY_NULL_PTR;

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
		else {
		    pop_node = Memory_NodeDispenser_Dispense(stack->node_dispenser);
		    if (pop_node == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;
		    pop_node->pointer = current->next->pointer;
		    pop_node->size = node_size;
		    pop_node->attrib = stack->attrib;
		    pop_node->next = MEMORY_NULL_PTR;
		
		    current->pointer += node_size * base_size;
		    (current->size) -= node_size;

			(stack->size) -= node_size;
			return pop_node;
		}
	}

    return pop_node;
}

struct Memory_Node* Memory_Stack_Get(struct Memory_Stack* stack, uintptr_t node_ptr) {
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

