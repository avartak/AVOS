#ifndef BOOT_SYSTEM_H
#define BOOT_SYSTEM_H

#include <bootloader/include/defs.h>

extern uint32_t System_StoreAPMInfo   (uint32_t addr);
extern uint32_t System_StoreSMBIOSInfo(uint32_t addr);
extern uint32_t System_StoreACPIInfo  (uint32_t addr, bool old);

#endif
