/*

Node attributes (32 bit field)

Bits  0-7   : 0xFF when the node is available
Bits  8-15  : Node size

*/


#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <kernel/include/common.h>

#define MEMORY_PHYSICAL_START_HIGHMEM  0x00400000

#define MEMORY_START_DISP              0xC0400000
#define MEMORY_END_DISP                0xC0800000
#define MEMORY_START_HEAP              0xC0800000
#define MEMORY_END_HEAP                0xE0000000

#define MEMORY_1B                      0x01
#define MEMORY_4B                      0x02
#define MEMORY_16B                     0x04
#define MEMORY_512B                    0x08
#define MEMORY_4KB                     0x10
#define MEMORY_4MB                     0x20

#define MEMORY_PAGE_MASK               (~0xFFF)

#define DISPENSARY_SIZE                0x400
#define DISPENSER_PRIME_ADDRESS        0xC0102000
#define DISPENSER_BOTTOM_OUT           8
#define DISPENSER_FULL_SIZE            (0x1000 - sizeof(struct Memory_NodeDispenser)) / sizeof(struct Memory_Node)
#define DISPENSER_FROM_NODE(node)      ((struct Memory_NodeDispenser*)((uintptr_t)node & (~0xFFF)))
#define DISPENSER_FIRST_NODE(disp)     ((uintptr_t)disp + sizeof(struct Memory_NodeDispenser))

struct Memory_RAM_Table_Entry {
	uintptr_t pointer;
	size_t    size;
}__attribute__((packed));

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

struct Memory_Map {
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

extern bool                Memory_Map_Contains(struct Memory_Map* stack, uintptr_t ptr_min, uintptr_t ptr_max);
extern bool                Memory_Map_Push    (struct Memory_Map* stack, struct Memory_Node* node, bool merge);
extern bool                Memory_Map_Append  (struct Memory_Map* stack, struct Memory_Node* node, bool merge);
extern bool                Memory_Map_Insert  (struct Memory_Map* stack, struct Memory_Node* node, bool merge);
extern struct Memory_Node* Memory_Map_Pop     (struct Memory_Map* stack);
extern struct Memory_Node* Memory_Map_Extract (struct Memory_Map* stack, size_t  node_size);
extern struct Memory_Node* Memory_Map_Get     (struct Memory_Map* stack, uintptr_t node_ptr); 


extern void                Memory_Physical_MakeMap       (struct Memory_Map* mem_map, uintptr_t mem_start, uintptr_t mem_end);
extern bool                Memory_Physical_AllocateBlock (uintptr_t virtual_address);
extern bool                Memory_Physical_AllocateBlocks(uintptr_t virtual_address, size_t nblocks);
extern bool                Memory_Physical_FreeBlock     (uintptr_t virtual_address);
extern bool                Memory_Physical_FreeBlocks    (uintptr_t virtual_address, size_t nblocks);
extern uintptr_t           Memory_Virtual_Allocate       (size_t nbytes);
extern bool                Memory_Virtual_Free           (uintptr_t pointer);

extern void                Memory_Initialize();

#endif
