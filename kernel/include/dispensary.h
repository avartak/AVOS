/*

Node attributes (32 bit field)

Bits  0-7   : 0xFF when the node is available
Bits  8-15  : Node size

*/


#ifndef KERNEL_DISPENSARY_H
#define KERNEL_DISPENSARY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MEMORY_NULL_PTR           ((void*)0xFFFFFFFF)
#define MEMORY_SIZE_PAGE          0x1000

#define VIRTUAL_MEMORY_START_HEAP 0xD0000000
#define VIRTUAL_MEMORY_END_HEAP   0xE0000000
#define VIRTUAL_MEMORY_START_DISP 0xC0400000
#define VIRTUAL_MEMORY_END_DISP   0xC0800000
#define SIZE_DISPENSARY           0x400

#define PAGE_MASK                 (~0xFFF)

#define MEMORY_1B                 0x01
#define MEMORY_4B                 0x02
#define MEMORY_16B                0x04
#define MEMORY_512B               0x08
#define MEMORY_4KB                0x10
#define MEMORY_4MB                0x20

#define MIN_NODES_B4_REFILL       8
#define FULL_DISPENSER_SIZE       (0x1000 - sizeof(struct Memory_NodeDispenser)) / sizeof(struct Memory_Node)
#define DISPENSER_FROM_NODE(node) ((struct Memory_NodeDispenser*)((uintptr_t)node & (~0xFFF)))
#define FIRST_NODE(disp)          ((uintptr_t)disp + sizeof(struct Memory_NodeDispenser))

struct Memory_Node {
	uintptr_t pointer;
	size_t    size;
	uint32_t  attrib;
	struct Memory_Node* next;
};

struct Memory_NodeDispenser {
	uintptr_t freenode;
	size_t    size;
	uint32_t  attrib;
	struct Memory_NodeDispenser* next;
};

extern size_t              Memory_Node_GetBaseSize(uint32_t attrib);

extern uintptr_t           Memory_NodeDispenser_New();
extern bool                Memory_NodeDispenser_Delete   (uintptr_t pointer);
extern bool                Memory_NodeDispenser_Return   (struct Memory_Node* node);
extern struct Memory_Node* Memory_NodeDispenser_Dispense (struct Memory_NodeDispenser* dispenser);
extern bool                Memory_NodeDispenser_Refill   (struct Memory_NodeDispenser* dispenser);
extern void                Memory_NodeDispenser_Retire   (struct Memory_NodeDispenser* dispenser);
extern size_t              Memory_NodeDispenser_NodesLeft(struct Memory_NodeDispenser* dispenser);
extern size_t              Memory_NodeDispenser_FullCount(struct Memory_NodeDispenser* dispenser);

#endif
