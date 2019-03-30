#ifndef X86_KERNEL_RAM_H
#define X86_KERNEL_RAM_H

#include <kernel/include/common.h>
#include <kernel/include/memory.h>

extern uintptr_t RAM_MaxPresentMemoryAddress();
extern bool      RAM_IsMemoryPresent(uintptr_t min, uintptr_t max);
extern bool      RAM_Initialize();

#endif
