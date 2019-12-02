#ifndef BOOT_X86_BIOS_DISCOVERY_H
#define BOOT_X86_BIOS_DISCOVERY_H

#include <boot/general/include/common.h>

extern uintptr_t Discovery_StoreAPMInfo   (uintptr_t addr);
extern uintptr_t Discovery_StoreSMBIOSInfo(uintptr_t addr);
extern uintptr_t Discovery_StoreACPIInfo  (uintptr_t addr, bool old);

#endif
