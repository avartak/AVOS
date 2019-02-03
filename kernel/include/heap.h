/*

Node attributes (32 bit field)

Bits  0-7   : 0xFF when the node is available, 0 when it is not
Bits  8-15  : Node size

*/


#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include <kernel/include/memory.h>
#include <x86/kernel/include/physmem.h>

#include <stdint.h>
#include <stdbool.h>

extern struct Memory_FIFO kernel_heap;

extern uintptr_t Heap_AllocatePage();
extern bool      Heap_FreePage(uintptr_t pointer);


#endif
