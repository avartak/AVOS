/*

Node attributes (32 bit field)

Bits  0-7   : 0xFF when the node is available, 0 when it is not
Bits  8-15  : Node size

*/


#ifndef KERNEL_CHUNK_H
#define KERNEL_CHUNK_H

#include <kernel/include/memory.h>

#include <stdint.h>
#include <stdbool.h>

extern struct    Memory_Stack Virtual_Memory_chunk;

extern uintptr_t Chunk_AllocatePage();
extern bool      Chunk_FreePage(uintptr_t pointer);

extern bool      Physical_Memory_AllocatePage(uintptr_t virtual_address);
extern bool      Physical_Memory_FreePage(uintptr_t virtual_address);


#endif
