/*

Node attributes (32 bit field)

Bits  0-7   : 0xFF when the node is available
Bits  8-15  : Node size

*/


#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_NULL_PTR           ((void*)0xFFFFFFFF)
#define MEMORY_SIZE_PAGE          0x1000

#define MEMORY_1B                 0x01
#define MEMORY_4B                 0x02
#define MEMORY_16B                0x04
#define MEMORY_512B               0x08
#define MEMORY_4KB                0x10
#define MEMORY_4MB                0x20

#define MIN_NODES_B4_REFILL       8
#define FULL_DISPENSER_SIZE       (0x1000 - sizeof(struct Memory_NodeDispenser)) / sizeof(struct Memory_Node)
#define DISPENSER_FROM_NODE(node) ((struct Memory_NodeDispenser*)((uintptr_t)node & (~0xFFF)))

struct Memory_Node {
	uintptr_t pointer;
	uint32_t  size;
	uint32_t  attrib;
	struct Memory_Node* next;
};

struct Memory_NodeDispenser {
	uintptr_t freenode;
	uint32_t  size;
	uint32_t  attrib;
	struct Memory_NodeDispenser* next;
};

struct Memory_Stack {
	struct Memory_Node* start;
	uint32_t size;
	uint32_t attrib;
	struct Memory_NodeDispenser* node_dispenser;
};

extern uint32_t            Memory_Node_GetBaseSize(uint32_t attrib);

extern bool                Memory_NodeDispenser_Return   (struct Memory_Node* node);
extern struct Memory_Node* Memory_NodeDispenser_Dispense (struct Memory_NodeDispenser* dispenser);
extern bool                Memory_NodeDispenser_Refill   (struct Memory_NodeDispenser* dispenser);
extern void                Memory_NodeDispenser_Retire   (struct Memory_NodeDispenser* dispenser);
extern uint32_t            Memory_NodeDispenser_NodesLeft(struct Memory_NodeDispenser* dispenser);
extern uint32_t            Memory_NodeDispenser_FullCount(struct Memory_NodeDispenser* dispenser);

extern bool                Memory_Stack_Push   (struct Memory_Stack* stack, struct Memory_Node* node);
extern bool                Memory_Stack_Append (struct Memory_Stack* stack, struct Memory_Node* node);
extern bool                Memory_Stack_Insert (struct Memory_Stack* stack, struct Memory_Node* node);
extern struct Memory_Node* Memory_Stack_Pop    (struct Memory_Stack* stack);
extern struct Memory_Node* Memory_Stack_Extract(struct Memory_Stack* stack, uint32_t node_size);

extern uintptr_t           Memory_AllocatePage();
extern bool                Memory_FreePage(uintptr_t pointer);

#endif
