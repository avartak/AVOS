#ifndef X86_KERNEL_PHYSMEM_H
#define X86_KERNEL_PHYSMEM_H

#include <kernel/include/memory.h>

#define VIRTUAL_MEMORY_HH_OFFSET      0xC0000000
#define VIRTUAL_MEMORY_E820_TABLE_PTR 0xC0010000
#define VIRTUAL_MEMORY_START_HEAP     0xD0000000
#define VIRTUAL_MEMORY_END_HEAP       0xE0000000

#define PHYSICAL_MEMORY_START_DMA     0x00800000
#define PHYSICAL_MEMORY_START_HIGHMEM 0x01000000
#define PHYSICAL_MEMORY_START_HEAP    0x00400000

struct E820_Table_Entry {
    uint32_t base;
    uint32_t base_high;
    uint32_t size;
    uint32_t size_high;
    uint32_t type;
    uint32_t acpi3;
}__attribute__((packed));

extern uint32_t  E820_Table_size;
extern struct    E820_Table_Entry* E820_Table;
extern struct    Memory_FIFO Physical_Memory_free;
extern struct    Memory_FIFO Physical_Memory_dma;
extern struct    Memory_FIFO kernel_heap;

extern void      Physical_Memory_Initialize();
extern bool      Physical_Memory_IsRangeFree(uintptr_t min, uintptr_t max, struct E820_Table_Entry* table, uint32_t size);
extern bool      Physical_Memory_IsPageFree(uintptr_t page, struct E820_Table_Entry* table, uint32_t size);
extern uint32_t  Physical_Memory_MaxFreeMemoryAddress(struct E820_Table_Entry* table, uint32_t size);

extern bool      Physical_Memory_AllocatePage(uintptr_t virtual_address);
extern bool      Physical_Memory_FreePage(uintptr_t virtual_address);

#endif
