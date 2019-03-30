#ifndef X86_KERNEL_RAM_H
#define X86_KERNEL_RAM_H

#include <kernel/include/common.h>

struct E820_Table_Entry {
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t acpi3;
}__attribute__((packed));

struct RAM_Table_Entry {
	uintptr_t pointer;
	size_t    size;
}__attribute__((packed));

extern uintptr_t RAM_MaxPresentMemoryAddress();
extern bool      RAM_IsMemoryPresent(uintptr_t min, uintptr_t max);
extern void      RAM_Initialize();

#endif
