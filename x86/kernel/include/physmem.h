#ifndef X86_KERNEL_PHYSMEM_H
#define X86_KERNEL_PHYSMEM_H

#include <kernel/include/common.h>
#include <kernel/include/memory.h>

#define PHYSICAL_MEMORY_START_DMA      0x00400000
#define PHYSICAL_MEMORY_START_HIGHMEM  0x01000000

struct E820_Table_Entry {
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t acpi3;
}__attribute__((packed));

extern size_t    E820_Table_size;
extern struct    E820_Table_Entry* E820_Table;
extern uint32_t* MBI_address;

extern struct    Memory_Stack Physical_Memory_usable;
extern struct    Memory_Stack Physical_Memory_high;
extern struct    Memory_Stack Physical_Memory_dma;

extern uint8_t   Kernel_dispensary_map[];
extern uint32_t  Kernel_dispensary_table[]__attribute__((aligned(0x1000)));
extern struct    Memory_NodeDispenser* Kernel_node_dispenser;

extern void      Physical_Memory_Initialize();
extern uintptr_t Physical_Memory_MaxFreeMemoryAddress(struct E820_Table_Entry* table, size_t size);
extern bool      Physical_Memory_IsMemoryAvailable(uintptr_t min, uintptr_t max, struct E820_Table_Entry* table, size_t size);
extern bool      Physical_Memory_CheckRange(uintptr_t min, uintptr_t max, uint32_t* mbi);
extern void      Physical_Memory_MakeMap(struct Memory_Stack* mem_map, uintptr_t map_start, uintptr_t map_end, struct E820_Table_Entry* table, size_t size);

extern bool      Physical_Memory_AllocatePage(uintptr_t virtual_address);
extern bool      Physical_Memory_FreePage(uintptr_t virtual_address);

#endif
