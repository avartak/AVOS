#ifndef BOOT_MEMORY_H
#define BOOT_MEMORY_H

#include <boot/include/defs.h>
#include <boot/include/multiboot.h>

#define MEMORY_E820_ACPI3_FLAG           1
#define MEMORY_E820_AVAILABLE            1
#define MEMORY_E820_RESERVED             2
#define MEMORY_E820_ACPI_RECLAIMABLE     3
#define MEMORY_E820_NVS                  4
#define MEMORY_E820_BADRAM               5

#define MEMORY_FIND_ADDRESS_BELOW        0
#define MEMORY_FIND_ADDRESS_ABOVE        1

extern uint32_t Memory_StoreInfo         (uint32_t addr, bool page_align, struct Multiboot_E820_Entry* E820_Table, uint32_t E820_Table_size);
extern uint32_t Memory_StoreE820Info     (uint32_t addr);
extern uint32_t Memory_StoreBasicInfo    (uint32_t addr);
extern uint32_t Memory_FindBlockAddress  (uint32_t addr, bool above, uint32_t size, uint32_t align, struct Boot_Block64* mmap, uint32_t mmap_size);

#endif
