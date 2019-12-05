#ifndef BOOT_X86_BIOS_RAM_H
#define BOOT_X86_BIOS_RAM_H

#include <boot/general/include/common.h>
#include <boot/general/include/multiboot.h>
#include <boot/general/include/boot.h>

#define RAM_HIGH_MEMORY_START   0x100000
#define RAM_32BIT_MEMORY_LIMIT  0xFFFFFFFF

extern uint32_t  RAM_MemoryBlockAboveAddress(uint32_t addr, uint32_t size, uint32_t align, struct Boot_Block64* mmap, size_t mmap_size);
extern uint32_t  RAM_MemoryBlockBelowAddress(uint32_t addr, uint32_t size, uint32_t align, struct Boot_Block64* mmap, size_t mmap_size);
extern uint64_t  RAM_MaxPresentMemoryAddress(struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size);
extern bool      RAM_IsMemoryPresent(uint64_t min, uint64_t max, struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size);
extern uintptr_t RAM_StoreInfo(uintptr_t addr, bool page_align, struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size);
extern uintptr_t RAM_StoreE820Info(uintptr_t addr);
extern uintptr_t RAM_StoreBasicInfo(uintptr_t addr);

#endif
