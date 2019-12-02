#ifndef BOOT_X86_BIOS_RAM_H
#define BOOT_X86_BIOS_RAM_H

#include <boot/general/include/common.h>
#include <boot/general/include/multiboot.h>

extern uint64_t  RAM_MaxPresentMemoryAddress(struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size);
extern bool      RAM_IsMemoryPresent(uint64_t min, uint64_t max, struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size);
extern uintptr_t RAM_StoreInfo(uintptr_t addr, struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size);
extern uintptr_t RAM_StoreE820Info(uintptr_t addr);
extern uintptr_t RAM_StoreBasicInfo(uintptr_t addr);

#endif
