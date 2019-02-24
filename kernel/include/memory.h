/*

Node attributes (32 bit field)

Bits  0-7   : 0xFF when the node is available
Bits  8-15  : Node size

*/


#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <kernel/include/common.h>
#include <kernel/include/dispensary.h>

struct Memory_Stack {
	struct Memory_Node* start;
	size_t   size;
	uint32_t attrib;
	struct Memory_NodeDispenser* node_dispenser;
};

extern bool                Memory_Stack_Contains(struct Memory_Stack* stack, uintptr_t ptr_min, uintptr_t ptr_max);
extern bool                Memory_Stack_Push    (struct Memory_Stack* stack, struct Memory_Node* node, bool merge);
extern bool                Memory_Stack_Append  (struct Memory_Stack* stack, struct Memory_Node* node, bool merge);
extern bool                Memory_Stack_Insert  (struct Memory_Stack* stack, struct Memory_Node* node, bool merge);
extern struct Memory_Node* Memory_Stack_Pop     (struct Memory_Stack* stack);
extern struct Memory_Node* Memory_Stack_Extract (struct Memory_Stack* stack, size_t  node_size);
extern struct Memory_Node* Memory_Stack_Get     (struct Memory_Stack* stack, uintptr_t node_ptr); 


#endif
