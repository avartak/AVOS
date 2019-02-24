#ifndef KERNEL_COMMON_H
#define KERNEL_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MEMORY_NULL_PTR           ((void*)0xFFFFFFFF)
#define MEMORY_SIZE_PAGE          0x1000

typedef uint32_t clock_t;

#endif
