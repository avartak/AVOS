#ifndef KERNEL_COMMON_H
#define KERNEL_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MEMORY_NULL_PTR           ((void*)0xFFFFFFFF)
#define MEMORY_SIZE_PAGE          0x1000
#define MEMORY_MAX_ADDRESS        0xFFFFFFFF

#define KERNEL_HIGHER_HALF_OFFSET 0xC0000000

#define BOOTINFO_ENTRY_E820       0
#define BOOTINFO_ENTRY_RAM        1
#define BOOTINFO_ENTRY_VBE        2

struct Info_Entry {
    uintptr_t address;
    size_t    size;
}__attribute__((packed));

typedef uint32_t clock_t;

#endif
