/*

Node attributes (32 bit field)

Bits  0-7   : 0xFF when the node is available
Bits  8-15  : Node size

*/


#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_NULL_PTR     ((void*)0xFFFFFFFF)
#define MEMORY_SIZE_PAGE    0x1000

#define MEMORY_1B           0x01
#define MEMORY_4B           0x02
#define MEMORY_16B          0x04
#define MEMORY_512B         0x08
#define MEMORY_4KB          0x10
#define MEMORY_4MB          0x20

#define MIN_NODES_B4_REFILL 8

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

struct Memory_FIFO {
	struct Memory_Node* start;
	uint32_t size;
	uint32_t attrib;
	struct Memory_NodeDispenser* node_dispenser;
};

static inline uint8_t      Memory_GetBaseSizeAttribute(uint32_t attrib);
static inline uint32_t     Memory_GetBaseSize(uint32_t attrib);

extern bool                Memory_NodeDispenser_Return   (struct Memory_Node* node);
extern struct Memory_Node* Memory_NodeDispenser_Dispense (struct Memory_NodeDispenser* dispenser);
extern bool                Memory_NodeDispenser_Refill   (struct Memory_NodeDispenser* dispenser);
extern bool                Memory_NodeDispenser_Retire   (struct Memory_NodeDispenser* dispenser);
extern uint32_t            Memory_NodeDispenser_NodesLeft(struct Memory_NodeDispenser* dispenser);

extern bool                Memory_Push  (struct Memory_FIFO* stack, struct Memory_Node* node);
extern struct Memory_Node* Memory_Pop   (struct Memory_FIFO* stack);
extern bool                Memory_Append(struct Memory_FIFO* stack, struct Memory_Node* node);

extern uintptr_t           Memory_AllocatePage();
extern bool                Memory_FreePage(uintptr_t pointer);

inline uint8_t Memory_GetBaseSizeAttribute(uint32_t attrib) {
	return (uint8_t)(attrib & 0xFF00 >> 8);
}

inline uint32_t Memory_GetBaseSize(uint32_t attrib) {
	uint8_t  base_size_attrib = Memory_GetBaseSizeAttribute(attrib);
	if      (base_size_attrib == 0x1 ) return 1;
	else if (base_size_attrib == 0x2 ) return 4;
	else if (base_size_attrib == 0x4 ) return 0x10;
	else if (base_size_attrib == 0x8 ) return 0x200;
	else if (base_size_attrib == 0x10) return 0x1000;
	else if (base_size_attrib == 0x20) return 0x400000;
	else return 0;
}

#endif
