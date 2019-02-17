#ifndef X86_KERNEL_PHYSMEM_H
#define X86_KERNEL_PHYSMEM_H

#include <kernel/include/memory.h>
#include <x86/kernel/include/e820.h>

#define PHYSICAL_MEMORY_START_DMA      0x00400000
#define PHYSICAL_MEMORY_START_HIGHMEM  0x01000000
#define PHYSICAL_MEMORY_START_DISP     0x01000000
#define PHYSICAL_MEMORY_START_HEAP     0x01100000

extern size_t    E820_Table_size;
extern struct    E820_Table_Entry* E820_Table;

extern struct    Memory_Stack Physical_Memory_high;
extern struct    Memory_Stack Physical_Memory_dma;

extern uint8_t   Kernel_lowlevel_heap[];
extern struct    Memory_NodeDispenser* Kernel_node_dispenser;

extern void      Physical_Memory_Initialize(uint32_t* mbi);
extern bool      Physical_Memory_IsRangeFree(uintptr_t min, uintptr_t max, struct E820_Table_Entry* table, size_t size);
extern uintptr_t Physical_Memory_MaxFreeMemoryAddress(struct E820_Table_Entry* table, size_t size);
extern void      Physical_Memory_MakeMap(struct Memory_Stack* mem_map, uintptr_t map_start, uintptr_t map_end, struct E820_Table_Entry* table, size_t size);

extern bool      Physical_Memory_AllocatePage(uintptr_t virtual_address);
extern bool      Physical_Memory_FreePage(uintptr_t virtual_address);

#endif
