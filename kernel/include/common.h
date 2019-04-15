#ifndef KERNEL_COMMON_H
#define KERNEL_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MEMORY_NULL_PTR           ((void*)0xFFFFFFFF)
#define MEMORY_SIZE_PAGE          0x1000
#define MEMORY_MAX_ADDRESS        0xFFFFFFFF

#define KERNEL_HIGHER_HALF_OFFSET 0xC0000000

#define BOOT_INFO_VBE_MAP_INDEX   0
#define BOOT_INFO_RAM_MAP_INDEX   2

typedef uint32_t clock_t;

#endif
