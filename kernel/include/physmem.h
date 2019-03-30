#ifndef KERNEL_PHYSMEM_H
#define KERNEL_PHYSMEM_H

#include <kernel/include/common.h>
#include <kernel/include/memory.h>

#define MEMORY_PHYSICAL_START_DMA      0x00400000
#define MEMORY_PHYSICAL_START_HIGHMEM  0x01000000

extern struct    Memory_Map Memory_Physical_high;
extern struct    Memory_Map Memory_Physical_dma;

extern uint8_t   Dispensary_pagemap[];
extern uint32_t  Dispensary_pagetable[]__attribute__((aligned(0x1000)));
extern struct    Memory_NodeDispenser* Node_dispenser;

extern bool      Memory_Physical_AllocateBlock(uintptr_t virtual_address);
extern bool      Memory_Physical_FreeBlock(uintptr_t virtual_address);
extern void      Memory_Physical_MakeMap(struct Memory_Map* mem_map, uintptr_t mem_start, uintptr_t mem_end);
extern void      Memory_Physical_Initialize();

#endif
