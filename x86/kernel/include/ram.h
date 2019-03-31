#ifndef X86_KERNEL_RAM_H
#define X86_KERNEL_RAM_H

#include <kernel/include/common.h>
#include <kernel/include/memory.h>

struct E820_Table_Entry {
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t acpi3;
}__attribute__((packed));

extern size_t E820_Table_size;
extern struct E820_Table_Entry* E820_Table;

extern size_t RAM_Table_size;
extern struct Memory_RAM_Table_Entry RAM_Table;

extern uintptr_t RAM_MaxPresentMemoryAddress();
extern bool      RAM_IsMemoryPresent(uintptr_t min, uintptr_t max);
extern bool      RAM_Initialize();


#endif
