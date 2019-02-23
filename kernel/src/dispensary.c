#include <kernel/include/dispensary.h>
#include <kernel/include/machine.h>

#include <stddef.h>

extern uint8_t Kernel_dispensary_map[];

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

	for (size_t i = 0; i < SIZE_DISPENSARY; i++) {
		if (Kernel_dispensary_map[i] == 0) { 
			Kernel_dispensary_map[i] = 1;
			pointer = VIRTUAL_MEMORY_START_DISP + i*MEMORY_SIZE_PAGE;
			ptridx  = i;
		}
	}

	if (pointer == (uintptr_t)MEMORY_NULL_PTR) return (uintptr_t)MEMORY_NULL_PTR;
    if (Memory_AllocateBlock(pointer) == false) {
        Kernel_dispensary_map[ptridx] = 0;
        return (uintptr_t)MEMORY_NULL_PTR;
    }

    return pointer;
}

bool Memory_NodeDispenser_Delete(uintptr_t pointer) {
	if (pointer < VIRTUAL_MEMORY_START_DISP || pointer >= VIRTUAL_MEMORY_END_DISP) return false;
	if (!Memory_FreeBlock(pointer)) return false;
	Kernel_dispensary_map[(pointer - VIRTUAL_MEMORY_START_DISP)/MEMORY_SIZE_PAGE] = 0;
	return true;
}

size_t Memory_NodeDispenser_NodesLeft(struct Memory_NodeDispenser* dispenser) {
	size_t size = 0;
	struct Memory_NodeDispenser* current_dispenser = dispenser;
	while (current_dispenser != MEMORY_NULL_PTR) {
		size += current_dispenser->size;
		current_dispenser = current_dispenser->next;
	}
	return size;
}

size_t Memory_NodeDispenser_FullCount(struct Memory_NodeDispenser* dispenser) {
    size_t count = 0;
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
	struct Memory_NodeDispenser* new_dispenser = (struct Memory_NodeDispenser*)Memory_NodeDispenser_New();
	if (new_dispenser == MEMORY_NULL_PTR) return false;

	struct Memory_NodeDispenser* current_dispenser = dispenser;
	while (current_dispenser->next != MEMORY_NULL_PTR) current_dispenser = current_dispenser->next;
	current_dispenser->next = new_dispenser;
	new_dispenser->freenode = FIRST_NODE(new_dispenser);
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
                    bool freed = Memory_NodeDispenser_Delete((uintptr_t)current_dispenser);
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

	size_t nodes_left = Memory_NodeDispenser_NodesLeft(dispenser);
	if (nodes_left == 0) return MEMORY_NULL_PTR;
	else {
		struct Memory_NodeDispenser* current_dispenser = dispenser;
		while (current_dispenser != MEMORY_NULL_PTR && (uintptr_t)(current_dispenser->freenode) == (uintptr_t)current_dispenser) current_dispenser = current_dispenser->next;
		if (current_dispenser == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;
		else {
			struct Memory_Node* return_node = (struct Memory_Node*)(current_dispenser->freenode);
			return_node->attrib &= (~0xFF);
			current_dispenser->freenode = (uintptr_t)current_dispenser;
			struct Memory_Node* nodes = (struct Memory_Node*)FIRST_NODE(current_dispenser);
			for (size_t i = 0; i < FULL_DISPENSER_SIZE; i++) {
				if ((nodes[i].attrib | 0xFF) == nodes[i].attrib) current_dispenser->freenode = (uintptr_t)(&(nodes[i]));
			}
			(current_dispenser->size)--;
			if (nodes_left < MIN_NODES_B4_REFILL) Memory_NodeDispenser_Refill(dispenser);
			return return_node;
		}
	}
}

