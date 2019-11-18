#ifndef X86_BOOT_DISCOVERY_H
#define X86_BOOT_DISCOVERY_H

#include <kernel/include/common.h>

extern uintptr_t Discovery_StoreAPMInfo   (uintptr_t addr);
extern uintptr_t Discovery_StoreSMBIOSInfo(uintptr_t addr);
extern uintptr_t Discovery_StoreACPIInfo  (uintptr_t addr, bool old);

#endif