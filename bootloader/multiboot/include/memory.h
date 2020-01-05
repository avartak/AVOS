/*
  
These functions are used for mapping the system RAM

* Memory_StoreBasicInfo      : Stores the size of low memory (correctly) and high memory (possibly underestimated) in KB
* Memory_StoreE820Info       : Stores the E820 memory map - returned by INT 0x15, AX=0xE820
* Memory_StoreInfo           : Stores a sanitized version (removing overlaps) of the E820 map. Only available regions are listed. Possible to align the start address on page boundary
* Memory_AlignAddress        : Find the nearest memory address either above or below a give location that conforms to the specified alignment
* Memory_FindBlockAddress    : Find memory chunk of a given size above/below an address, possibly aligned to a certain value

*/


#ifndef BOOTLOADER_MEMORY_H
#define BOOTLOADER_MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include <bootloader/initial/include/bootdefs.h>
#include <bootloader/multiboot/include/multiboot.h>

#define MEMORY_NULL_PTR                  ((void*)0xFFFFFFFF)
#define MEMORY_32BIT_LIMIT               0xFFFFFFFF
#define MEMORY_HIGH_START                0x100000

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
extern uint32_t Memory_AlignAddress      (uint32_t addr, uint32_t align, bool above) ;
extern uint32_t Memory_FindBlockAddress  (uint32_t addr, bool above, uint32_t size, uint32_t align, struct BootInfo_BlockRAM* mmap, uint32_t mmap_size);

#endif
