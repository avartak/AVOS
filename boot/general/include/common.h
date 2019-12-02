#ifndef BOOT_GENERAL_COMMON_H
#define BOOT_GENERAL_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MEMORY_NULL_PTR           ((void*)0xFFFFFFFF)

struct Block32_Entry {
    uintptr_t address;
    size_t    size;
}__attribute__((packed));

struct Block64_Entry {
    uint64_t address;
    uint64_t    size;
}__attribute__((packed));

#endif
