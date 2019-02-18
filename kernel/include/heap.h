/*

Node attributes (32 bit field)

Bits  0-7   : 0xFF when the node is available, 0 when it is not
Bits  8-15  : Node size

*/


#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include <kernel/include/memory.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

extern uintptr_t KHeap_Allocate(size_t nbytes);
extern bool      KHeap_Free(uintptr_t pointer);
extern void      KHeap_Initialize(uintptr_t vm_start, uintptr_t vm_end);

#endif
