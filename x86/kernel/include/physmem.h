#ifndef X86_KERNEL_PHYSMEM_H
#define X86_KERNEL_PHYSMEM_H

#include <kernel/include/memory.h>
#include <x86/kernel/include/e820.h>

#define VIRTUAL_MEMORY_E820_TABLE_PTR  0xC0010000
#define VIRTUAL_MEMORY_START_CHUNK     0xD0000000
#define VIRTUAL_MEMORY_END_CHUNK       0xD0400000

#define PHYSICAL_MEMORY_START_DMA      0x00800000
#define PHYSICAL_MEMORY_START_HIGHMEM  0x01000000
#define PHYSICAL_MEMORY_START_CHUNK    0x00400000

extern struct    Memory_Stack Physical_Memory_free;
extern struct    Memory_Stack Physical_Memory_dma;
extern struct    Memory_Stack Virtual_Memory_free;
extern struct    Memory_Stack Virtual_Memory_inuse;

extern void      Memory_Initialize(uint32_t* mbi);
extern bool      Physical_Memory_IsRangeFree(uintptr_t min, uintptr_t max, struct E820_Table_Entry* table, uint32_t size);
extern uint32_t  Physical_Memory_MaxFreeMemoryAddress(struct E820_Table_Entry* table, uint32_t size);
extern void      Physical_Memory_MakeMap(struct Memory_Stack* mem_map, uintptr_t map_start, uintptr_t map_end, struct E820_Table_Entry* table, uint32_t size);

extern bool      Physical_Memory_AllocatePage(uintptr_t virtual_address);
extern bool      Physical_Memory_FreePage(uintptr_t virtual_address);

#endif
