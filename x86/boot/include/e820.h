#ifndef X86_BOOT_E820_H
#define X86_BOOT_E820_H

#include <kernel/include/common.h>
#include <kernel/include/multiboot.h>

extern uintptr_t E820_StoreInfo(uintptr_t addr);

#endif
