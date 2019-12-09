#ifndef BOOT_MEMORY_H
#define BOOT_MEMORY_H

#include <boot/include/defs.h>
#include <boot/include/multiboot.h>

extern uint64_t  Memory_MaxPhysicalAddress(struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size);
extern bool      Memory_IsPresent         (uint64_t min, uint64_t max, struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size);
extern uintptr_t Memory_StoreInfo         (uintptr_t addr, bool page_align, struct Multiboot_E820_Entry* E820_Table, size_t E820_Table_size);
extern uintptr_t Memory_StoreE820Info     (uintptr_t addr);
extern uintptr_t Memory_StoreBasicInfo    (uintptr_t addr);

extern uint32_t  Memory_BlockAboveAddress (uint32_t addr, uint32_t size, uint32_t align, struct Boot_Block64* mmap, size_t mmap_size);
extern uint32_t  Memory_BlockBelowAddress (uint32_t addr, uint32_t size, uint32_t align, struct Boot_Block64* mmap, size_t mmap_size);

#endif
