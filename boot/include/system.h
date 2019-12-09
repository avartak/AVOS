#ifndef BOOT_SYSTEM_H
#define BOOT_SYSTEM_H

#include <boot/include/defs.h>

extern uintptr_t System_StoreAPMInfo   (uintptr_t addr);
extern uintptr_t System_StoreSMBIOSInfo(uintptr_t addr);
extern uintptr_t System_StoreACPIInfo  (uintptr_t addr, bool old);

#endif
