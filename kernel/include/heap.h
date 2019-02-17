/*

Node attributes (32 bit field)

Bits  0-7   : 0xFF when the node is available, 0 when it is not
Bits  8-15  : Node size

*/


#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include <kernel/include/memory.h>

#include <stdint.h>
#include <stdbool.h>

#define VIRTUAL_MEMORY_START_HEAP      0xD0100000
#define VIRTUAL_MEMORY_END_HEAP        0xE0000000

extern uintptr_t Heap_Allocate(uint32_t nbytes);
extern bool      Heap_Free(uintptr_t pointer);
extern void      Heap_Initialize();

#endif
