#ifndef X86_KERNEL_PHYSMEM_H
#define X86_KERNEL_PHYSMEM_H

#include <kernel/include/common.h>
#include <kernel/include/memory.h>
#include <x86/kernel/include/ram.h>

#define PHYSICAL_MEMORY_START_DMA      0x00400000
#define PHYSICAL_MEMORY_START_HIGHMEM  0x01000000

extern struct    Memory_Stack Physical_Memory_high;
extern struct    Memory_Stack Physical_Memory_dma;

extern uint8_t   Kernel_dispensary_map[];
extern uint32_t  Kernel_dispensary_table[]__attribute__((aligned(0x1000)));
extern struct    Memory_NodeDispenser* Kernel_node_dispenser;

extern void      Physical_Memory_Initialize();
extern bool      Physical_Memory_AllocatePage(uintptr_t virtual_address);
extern bool      Physical_Memory_FreePage(uintptr_t virtual_address);
extern void      Physical_Memory_MakeMap(struct Memory_Stack* mem_map, uintptr_t mem_start, uintptr_t mem_end);

#endif
