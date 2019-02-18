#include <kernel/include/memory.h>

#include <stddef.h>

bool Memory_Stack_Contains(struct Memory_Stack* stack, uintptr_t ptr_min, uintptr_t ptr_max) {
    if (stack == MEMORY_NULL_PTR) return false;

	struct Memory_Node* current = stack->start;
	while (current != MEMORY_NULL_PTR) {
		if (ptr_min >= current->pointer && ptr_max <= current->pointer + current->size * Memory_Node_GetBaseSize(stack->attrib)) return true;
		current = current->next;
	}

	return false;
}


bool Memory_Stack_Push(struct Memory_Stack* stack, struct Memory_Node* node, bool merge) {
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

bool Memory_Stack_Append(struct Memory_Stack* stack, struct Memory_Node* node, bool merge) {
    if (stack == MEMORY_NULL_PTR || node == MEMORY_NULL_PTR) return false;
    if (node->attrib != stack->attrib) return false;

    size_t node_base_size = Memory_Node_GetBaseSize(stack->attrib);
    uintptr_t node_end = node->pointer + node->size * node_base_size;
    if (node_end <= node->pointer) return false;

	struct Memory_Node* last_node = stack->start;
	if (last_node == MEMORY_NULL_PTR) return Memory_Stack_Push(stack, node, merge);
	while (last_node->next != MEMORY_NULL_PTR) last_node = last_node->next;

    if (last_node->pointer + last_node->size * node_base_size != node->pointer || !merge) {
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

bool Memory_Stack_Insert(struct Memory_Stack* stack, struct Memory_Node* node, bool merge) {
    if (stack == MEMORY_NULL_PTR || node == MEMORY_NULL_PTR) return false;
    if (node->attrib != stack->attrib) return false;

    size_t node_base_size = Memory_Node_GetBaseSize(stack->attrib);
    uintptr_t node_end = node->pointer + node->size * node_base_size;
    if (node_end <= node->pointer) return false;

	if (stack->start == MEMORY_NULL_PTR || node_end <= stack->start->pointer) return Memory_Stack_Push(stack, node, merge);

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

	if (current_node->next == MEMORY_NULL_PTR && current_node->pointer + current_node->size * node_base_size <= node->pointer) return Memory_Stack_Append(stack, node, merge);

	return false;
}

struct Memory_Node* Memory_Stack_Pop(struct Memory_Stack* stack) {
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

struct Memory_Node* Memory_Stack_Extract(struct Memory_Stack* stack, size_t node_size) {
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

