#ifndef BOOTLOADER_SYSTEM_H
#define BOOTLOADER_SYSTEM_H

#include <bootloader/multiboot/include/defs.h>

extern uint32_t System_StoreAPMInfo   (uint32_t addr);
extern uint32_t System_StoreSMBIOSInfo(uint32_t addr);
extern uint32_t System_StoreACPIInfo  (uint32_t addr, bool old);

#endif
