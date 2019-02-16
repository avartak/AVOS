#ifndef X86_KERNEL_KINIT_H
#define X86_KERNEL_KINIT_H

#include <x86/kernel/include/e820.h>

#include <stdint.h>
#include <stdbool.h>

#define CHECK_MEMORY_START      0x100000
#define CHECK_MEMORY_END        0x800000

extern bool Initial_Checks(uint32_t* mbi)__attribute__((section ("start")));
extern bool IsMemoryAvailable(uintptr_t min, uintptr_t max, struct E820_Table_Entry* table, uint32_t size)__attribute__((section ("start")));

#endif
