/*

Node attributes (32 bit field)

Bits  0-7   : 0xFF when the node is available, 0 when it is not
Bits  8-15  : Node size

*/


#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include <kernel/include/common.h>

extern uintptr_t Heap_Allocate(size_t nbytes);
extern bool      Heap_Free(uintptr_t pointer);
extern void      Heap_Initialize();

#endif
