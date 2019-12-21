/*
  
These functions are used for mapping the system RAM

* Memory_StoreBasicInfo      : Stores the size of low memory (correctly) and high memory (possibly underestimated) in KB
* Memory_StoreE820Info       : Stores the E820 memory map - returned by INT 0x15, AX=0xE820
* Memory_StoreInfo           : Stores a sanitized version (removing overlaps) of the E820 map. Only available regions are listed. Possible to align the start address on page boundary
* Memory_FindBlockAddress    : Find memory chunk of a given size above/below an address, possibly aligned to a certain value

*/


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
