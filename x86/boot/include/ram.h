#ifndef X86_BOOT_RAM_H
#define X86_BOOT_RAM_H

#include <kernel/include/common.h>

extern uintptr_t RAM_MaxPresentMemoryAddress();
extern bool      RAM_IsMemoryPresent(uintptr_t min, uintptr_t max);
extern uintptr_t RAM_StoreInfo(uintptr_t addr);

#endif
