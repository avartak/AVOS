/*

Node attributes (32 bit field)

Bits  0-7   : 0xFF when the node is available
Bits  8-15  : Node size

*/


#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <kernel/include/common.h>

#define MEMORY_PHYSICAL_START_DMA      0x00400000
#define MEMORY_PHYSICAL_START_HIGHMEM  0x01000000

#define MEMORY_VIRTUAL_START_HEAP      0xD0000000
#define MEMORY_VIRTUAL_END_HEAP        0xE0000000
#define MEMORY_VIRTUAL_START_DISP      0xC0400000
#define MEMORY_VIRTUAL_END_DISP        0xC0800000

#define MEMORY_1B                      0x01
#define MEMORY_4B                      0x02
#define MEMORY_16B                     0x04
#define MEMORY_512B                    0x08
#define MEMORY_4KB                     0x10
#define MEMORY_4MB                     0x20

#define MEMORY_PAGE_MASK               (~0xFFF)

#define DISPENSARY_SIZE                0x400
#define DISPENSER_BOTTOM_OUT           8
#define DISPENSER_FULL_SIZE            (0x1000 - sizeof(struct Memory_NodeDispenser)) / sizeof(struct Memory_Node)
#define DISPENSER_FROM_NODE(node)      ((struct Memory_NodeDispenser*)((uintptr_t)node & (~0xFFF)))
#define DISPENSER_FIRST_NODE(disp)     ((uintptr_t)disp + sizeof(struct Memory_NodeDispenser))


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

struct Memory_Stack {
	struct Memory_Node* start;
	size_t   size;
	uint32_t attrib;
	struct Memory_NodeDispenser* node_dispenser;
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

extern bool                Memory_Stack_Contains(struct Memory_Stack* stack, uintptr_t ptr_min, uintptr_t ptr_max);
extern bool                Memory_Stack_Push    (struct Memory_Stack* stack, struct Memory_Node* node, bool merge);
extern bool                Memory_Stack_Append  (struct Memory_Stack* stack, struct Memory_Node* node, bool merge);
extern bool                Memory_Stack_Insert  (struct Memory_Stack* stack, struct Memory_Node* node, bool merge);
extern struct Memory_Node* Memory_Stack_Pop     (struct Memory_Stack* stack);
extern struct Memory_Node* Memory_Stack_Extract (struct Memory_Stack* stack, size_t  node_size);
extern struct Memory_Node* Memory_Stack_Get     (struct Memory_Stack* stack, uintptr_t node_ptr); 


#endif
