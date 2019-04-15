#ifndef X86_BOOT_E820_H
#define X86_BOOT_E820_H

#include <kernel/include/common.h>

struct E820_Table_Entry {
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t acpi3;
}__attribute__((packed));

extern size_t E820_Table_size;
extern struct E820_Table_Entry* E820_Table;

extern uintptr_t E820_StoreInfo(uintptr_t addr);

#endif
