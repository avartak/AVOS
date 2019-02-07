#ifndef X86_KERNEL_E820_H
#define X86_KERNEL_E820_H

#include <stdint.h>

struct E820_Table_Entry {
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t acpi3;
}__attribute__((packed));

extern uint32_t  E820_Table_size;
extern struct    E820_Table_Entry* E820_Table;

#endif
